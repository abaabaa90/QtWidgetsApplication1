void vtkTecplotReader::ReadFile(vtkMultiBlockDataSet* multZone)
{
    int zoneIndex = 0;
    bool firstToken = true;
    bool tokenReady = false;

    this->Init();
    this->Internal->ASCIIStream.open(this->FileName);
    std::string tok = this->Internal->GetNextToken();

    while (!this->Internal->NextCharEOF)
    {
        tokenReady = false;
        if (tok.empty())
        {
            // whitespace: do nothing
        }
        else if (tok == "TITLE")
        {
            this->DataTitle = this->Internal->GetNextToken();
        }
        else if (tok == "GEOMETRY")
        {
            // unsupported
            tok = this->Internal->GetNextToken();
            while (READ_UNTIL_LINE_END)
            {
                // skipping token
                tok = this->Internal->GetNextToken();
            }
            tokenReady = true;
        }
        else if (tok == "TEXT")
        {
            // unsupported
            tok = this->Internal->GetNextToken();
            while (READ_UNTIL_LINE_END)
            {
                // Skipping token
                tok = this->Internal->GetNextToken();
            }
            tokenReady = true;
        }
        else if (tok == "VARIABLES")
        {
            int guessedXindex = -1;
            int guessedYindex = -1;
            int guessedZindex = -1;

            // variable lists
            tok = this->Internal->GetNextToken();
            while (this->Internal->TokenIsString)
            {
                int tokLen = int(tok.length());
                for (int i = 0; i < tokLen; i++)
                {
                    if (tok[i] == '(')
                    {
                        tok[i] = '[';
                    }
                    else if (tok[i] == ')')
                    {
                        tok[i] = ']';
                    }
                    else if (tok[i] == '/')
                    {
                        tok[i] = '_';
                    }
                }

                std::string tok_nw = SimplifyWhitespace(tok);

                switch (GetCoord(tok_nw))
                {
                case 0:
                    this->Internal->XIdInList = this->NumberOfVariables;
                    break;
                case 1:
                    this->Internal->YIdInList = this->NumberOfVariables;
                    break;
                case 2:
                    this->Internal->ZIdInList = this->NumberOfVariables;
                    break;
                default:
                    break;
                }

                switch (GuessCoord(tok_nw))
                {
                case 0:
                    guessedXindex = this->NumberOfVariables;
                    break;
                case 1:
                    guessedYindex = this->NumberOfVariables;
                    break;
                case 2:
                    guessedZindex = this->NumberOfVariables;
                    break;
                default:
                    break;
                }

                this->Variables.push_back(tok);
                this->NumberOfVariables++;
                tok = this->Internal->GetNextToken();
            }

            if (this->NumberOfVariables == 0)
            {
                while (true)
                {
                    std::string tok_nw = SimplifyWhitespace(tok);

                    switch (GetCoord(tok_nw))
                    {
                    case 0:
                        this->Internal->XIdInList = this->NumberOfVariables;
                        break;
                    case 1:
                        this->Internal->YIdInList = this->NumberOfVariables;
                        break;
                    case 2:
                        this->Internal->ZIdInList = this->NumberOfVariables;
                        break;
                    default:
                        break;
                    }

                    switch (GuessCoord(tok_nw))
                    {
                    case 0:
                        guessedXindex = this->NumberOfVariables;
                        break;
                    case 1:
                        guessedYindex = this->NumberOfVariables;
                        break;
                    case 2:
                        guessedZindex = this->NumberOfVariables;
                        break;
                    default:
                        break;
                    }

                    this->Variables.push_back(tok);
                    this->NumberOfVariables++;

                    if (this->Internal->NextCharEOL)
                    {
                        tok = this->Internal->GetNextToken();
                        break;
                    }
                    else
                    {
                        tok = this->Internal->GetNextToken();
                    }
                }
            }

            // Default the centering to nodal
            this->CellBased.clear();
            this->CellBased.resize(this->NumberOfVariables, 0);

            // If we didn't find an exact match for coordinate axis vars, guess
            if (this->Internal->XIdInList < 0)
            {
                this->Internal->XIdInList = guessedXindex;
            }
            if (this->Internal->YIdInList < 0)
            {
                this->Internal->YIdInList = guessedYindex;
            }
            if (this->Internal->ZIdInList < 0)
            {
                this->Internal->ZIdInList = guessedZindex;
            }

            // Based on how many spatial coords we got, guess the spatial dim
            if (this->Internal->XIdInList >= 0)
            {
                this->Internal->GeometryDim = 1;
                if (this->Internal->YIdInList >= 0)
                {
                    this->Internal->GeometryDim = 2;
                    if (this->Internal->ZIdInList >= 0)
                    {
                        this->Internal->GeometryDim = 3;
                    }
                }
            }

            tokenReady = true;
        }
        else if (tok == "ZONE")
        {
            int numI = 1;
            int numJ = 1;
            int numK = 1;
            int numNodes = 0;
            int numFaces = 0;
            int numConnectedBoundaryFaces(-1);
            int totalNumBoundaryConnections(-1);
            int numElements = 0;

            std::string format;
            std::string elemType;
            std::string zoneType;
            std::string ZoneName = vtk::format("zone{:05d}", zoneIndex);
            tok = this->Internal->GetNextToken();
            // instead of looking for known keywords, read the zone header until the first numeric token
            while (tok.front() != '-' && tok.front() != '.' && !isdigit(tok.front()))
            {
                if (tok == "T")
                {
                    ZoneName = this->Internal->GetNextToken();
                    if (!this->Internal->TokenIsString)
                    {
                        vtkErrorMacro(<< this->FileName << ": Zone titles MUST be "
                            << "quoted.");
                        return;
                    }
                }
                else if (tok == "I")
                {
                    VTK_FROM_CHARS_IF_ERROR_RETURN(this->Internal->GetNextToken(), numI, );
                }
                else if (tok == "J")
                {
                    VTK_FROM_CHARS_IF_ERROR_RETURN(this->Internal->GetNextToken(), numJ, );
                }
                else if (tok == "K")
                {
                    VTK_FROM_CHARS_IF_ERROR_RETURN(this->Internal->GetNextToken(), numK, );
                }
                else if (tok == "N" || tok == "NODES")
                {
                    VTK_FROM_CHARS_IF_ERROR_RETURN(this->Internal->GetNextToken(), numNodes, );
                }
                else if (tok == "E" || tok == "ELEMENTS")
                {
                    VTK_FROM_CHARS_IF_ERROR_RETURN(this->Internal->GetNextToken(), numElements, );
                }
                else if (tok == "ET")
                {
                    elemType = this->Internal->GetNextToken();
                }
                else if (tok == "ZONETYPE")
                {
                    zoneType = this->Internal->GetNextToken();
                }
                else if (tok == "F" || tok == "DATAPACKING")
                {
                    format = this->Internal->GetNextToken();
                }
                else if (tok == "VARLOCATION")
                {
                    std::string centering;
                    this->CellBased.clear();
                    this->CellBased.resize(this->NumberOfVariables, 0);

                    // read token to ascertain VARLOCATION syntax usage
                    std::string var_format_type = this->Internal->GetNextToken();

                    // if each variable will have data type specified explicitly (as is handled in old Tecplot
                    // reader), else a range is specified for CELLCENTERED only, with NODAL values assumed
                    // implicitly
                    if (var_format_type == "NODAL" || var_format_type == "CELLCENTERED")
                    {
                        if (var_format_type == "CELLCENTERED")
                        {
                            this->CellBased[0] = 1;
                        }
                        for (int i = 1; i < this->NumberOfVariables; i++)
                        {
                            centering = this->Internal->GetNextToken();
                            if (centering == "CELLCENTERED")
                            {
                                this->CellBased[i] = 1;
                            }
                        }
                    }
                    else
                    {
                        do
                        {
                            // remove left square bracket, if it exists
                            size_t brack_pos = var_format_type.find('[');
                            if (brack_pos != std::string::npos)
                                var_format_type.erase(brack_pos, brack_pos + 1);

                            // remove right square bracket, if it exists
                            brack_pos = var_format_type.find(']');
                            if (brack_pos != std::string::npos)
                                var_format_type.erase(brack_pos, brack_pos + 1);

                            // if a range is defined, then split again, convert to int and set to cell data
                            // else if a single value is defined, then just set the flag directly
                            if (var_format_type.find('-') != std::string::npos)
                            {
                                std::vector<std::string> var_range;
                                vtksys::SystemTools::Split(var_format_type, var_range, '-');

                                int cell_start, cell_end;
                                VTK_FROM_CHARS_IF_ERROR_RETURN(var_range[0], cell_start, );
                                --cell_start; // convert from FORTRAN to C-indexing
                                VTK_FROM_CHARS_IF_ERROR_RETURN(var_range[1], cell_end, );
                                for (int i = cell_start; i != cell_end; ++i)
                                {
                                    this->CellBased[i] = 1;
                                }
                            }
                            else
                            {
                                int index;
                                VTK_FROM_CHARS_IF_ERROR_RETURN(var_format_type, index, );
                                --index; // convert from FORTRAN to C-indexing
                                this->CellBased[index] = 1;
                            }

                            // get next value
                            var_format_type = this->Internal->GetNextToken();

                            // continue until the CELLCENTERED keyword is found
                        } while (var_format_type != "CELLCENTERED");
                    }
                }
                else if (tok == "DT")
                {
                    for (int i = 0; i < this->NumberOfVariables; i++)
                    {
                        this->Internal->GetNextToken();
                    }
                }
                else if (tok == "D")
                {
                    vtkWarningMacro(<< this->FileName << "; Tecplot zone record parameter "
                        << "'D' is currently unsupported.");
                    this->Internal->GetNextToken();
                }
                else if (tok == "STRANDID")
                {
                    vtkWarningMacro(<< this->FileName << "; Tecplot zone record parameter "
                        << "'STRANDID' is currently unsupported.");
                    this->Internal->GetNextToken();
                }
                else if (tok == "SOLUTIONTIME")
                {
                    vtkWarningMacro(<< this->FileName << "; Tecplot zone record parameter "
                        << "'SOLUTIONTIME' is currently unsupported.");
                    this->Internal->GetNextToken();
                }
                else if (tok == "PARENTZONE")
                {
                    vtkWarningMacro(<< this->FileName << "; Tecplot zone record parameter "
                        << "'PARENTZONE' is currently unsupported.");
                    this->Internal->GetNextToken();
                }
                else if (tok == "AUXDATA")
                {
                    while (READ_UNTIL_LINE_END)
                    {
                        // Skipping token
                        tok = this->Internal->GetNextToken();

                        // the READ_UNTIL_LINE_END macro does NOT read until a line ends
                        // but it reads until a next known keyword is encountered.
                        if (this->Internal->NextCharEOL)
                        {
                            break;
                        }
                    }
                }
                else if (tok == "FACES")
                {
                    VTK_FROM_CHARS_IF_ERROR_RETURN(this->Internal->GetNextToken(), numFaces, );
                }
                else if (tok == "TOTALNUMFACENODES")
                {
                    // this parameter is not used
                    this->Internal->GetNextToken();
                }
                else if (tok == "NUMCONNECTEDBOUNDARYFACES")
                {
                    VTK_FROM_CHARS_IF_ERROR_RETURN(
                        this->Internal->GetNextToken(), numConnectedBoundaryFaces, );
                    if (0 != numConnectedBoundaryFaces)
                    {
                        vtkWarningMacro(<< "Non-zero number of connected boundary faces is not supported.");
                    }
                }
                else if (tok == "TOTALNUMBOUNDARYCONNECTIONS")
                {
                    VTK_FROM_CHARS_IF_ERROR_RETURN(
                        this->Internal->GetNextToken(), totalNumBoundaryConnections, );
                    if (0 != totalNumBoundaryConnections)
                    {
                        vtkWarningMacro(<< "Non-zero number of total #boundary faces is not supported.");
                    }
                }
                else
                {
                    vtkDebugMacro(<< this->FileName << "; encountered an unknown token: '" << tok
                        << "'. This will be skipped.");
                }
                tok = this->Internal->GetNextToken();
            } // end while loop looking for known tokens

            this->Internal->TokenBackup = tok;

            this->ZoneNames.push_back(ZoneName);

            if (zoneType.empty())
            {
                if (format == "FEBLOCK")
                {
                    this->GetUnstructuredGridFromBlockPackingZone(
                        numNodes, numElements, elemType.c_str(), zoneIndex, ZoneName.c_str(), multZone);
                }
                else if (format == "FEPOINT")
                {
                    this->GetUnstructuredGridFromPointPackingZone(
                        numNodes, numElements, elemType.c_str(), zoneIndex, ZoneName.c_str(), multZone);
                }
                else if (format == "BLOCK")
                {
                    this->GetStructuredGridFromBlockPackingZone(
                        numI, numJ, numK, zoneIndex, ZoneName.c_str(), multZone);
                }
                else if (format == "POINT")
                {
                    this->GetStructuredGridFromPointPackingZone(
                        numI, numJ, numK, zoneIndex, ZoneName.c_str(), multZone);
                }
                else if (format.empty())
                {
                    // No format given; we will assume we got a POINT format
                    this->GetStructuredGridFromPointPackingZone(
                        numI, numJ, numK, zoneIndex, ZoneName.c_str(), multZone);
                }
                else
                {
                    // UNKNOWN FORMAT
                    vtkErrorMacro(<< this->FileName << ": The format " << format
                        << " found in the file is unknown.");
                    return;
                }
            }
            else
            {
                if (zoneType == "ORDERED")
                {
                    if (format == "POINT")
                    {
                        this->GetStructuredGridFromPointPackingZone(
                            numI, numJ, numK, zoneIndex, ZoneName.c_str(), multZone);
                    }
                    else if (format == "BLOCK")
                    {
                        this->GetStructuredGridFromPointPackingZone(
                            numI, numJ, numK, zoneIndex, ZoneName.c_str(), multZone);
                    }
                }
                else if (zoneType == "FETRIANGLE" || zoneType == "FEQUADRILATERAL" ||
                    zoneType == "FEBRICK" || zoneType == "FETETRAHEDRON")
                {
                    std::string elType = zoneType.substr(2);
                    if (format == "POINT")
                    {
                        this->GetUnstructuredGridFromPointPackingZone(
                            numNodes, numElements, elType.c_str(), zoneIndex, ZoneName.c_str(), multZone);
                    }
                    else if (format == "BLOCK")
                    {
                        this->GetUnstructuredGridFromBlockPackingZone(
                            numNodes, numElements, elType.c_str(), zoneIndex, ZoneName.c_str(), multZone);
                    }
                }
                else if (zoneType == "FEPOLYHEDRON")
                {
                    this->GetPolyhedralGridFromBlockPackingZone(
                        numNodes, numElements, numFaces, zoneIndex, ZoneName.c_str(), multZone);
                }
                else if (zoneType == "FEPOLYGON")
                {
                    this->GetPolygonalGridFromBlockPackingZone(
                        numNodes, numElements, numFaces, zoneIndex, ZoneName.c_str(), multZone);
                }
                else
                {
                    vtkWarningMacro(<< " ZONETYPE '" << zoneType << "' is currently unsupported.");
                }
            }

            zoneIndex++;
        }
        else if (tok == "DATASETAUXDATA")
        {
            int tokIndex = 0;
            bool haveVectorExpr = false;
            tok = this->Internal->GetNextToken();

            while (READ_UNTIL_LINE_END)
            {
                if (tokIndex == 0)
                {
                    haveVectorExpr = (tok == "VECTOR");
                }
                else if (tokIndex == 1)
                {
                    if (haveVectorExpr)
                    {
                        // Remove spaces
                        std::string::size_type pos = tok.find(' ');
                        while (pos != std::string::npos)
                        {
                            tok.replace(pos, 1, "");
                            pos = tok.find(' ');
                        }

                        // Look for '('
                        pos = tok.find('(');
                        if (pos != std::string::npos)
                        {
#ifndef NDEBUG
                            std::string exprName(tok.substr(0, pos));
#endif
                            std::string exprDef(tok.substr(pos, tok.size() - pos));

                            exprDef.replace(0, 1, "{");

                            // Replace ')' with '}'
                            pos = exprDef.find(')');
                            if (pos != std::string::npos)
                            {
                                exprDef.replace(pos, 1, "}");
                                vtkDebugMacro("Expr name = " << exprName << ", Expr def = " << exprDef);
                            }
                        }
                    }
                }

                // Skipping token
                tok = this->Internal->GetNextToken();
                tokIndex++;
            }

            tokenReady = true;
        }
        else if (firstToken && this->Internal->TokenIsString)
        {
            // Robust: assume it's a title
            this->DataTitle = tok;
        }
        else
        {
            // UNKNOWN RECORD TYPE
            vtkErrorMacro(<< this->FileName << ": The record type " << tok
                << " found in the file is unknown.");
            return;
        }

        firstToken = false;
        if (!tokenReady)
        {
            tok = this->Internal->GetNextToken();
        }
    }
    this->Internal->ASCIIStream.close();

    this->Internal->TopologyDim = std::min(this->Internal->TopologyDim, this->Internal->GeometryDim);

    this->Internal->Completed = 1;
}

//------------------------------------------------------------------------------
void vtkTecplotReader::GetArraysFromPointPackingZone(
    int numNodes, vtkPoints* theNodes, vtkPointData* nodeData)
{
    // NOTE: The Tecplot ASCII file format mandates that cell data of any zone be
    // stored in block-packing mode (VARLOCATION, pp. 158, Tecplot 360 Data Format
    // Guide 2009). Thus we do not need to consider any cell data in this function.

    if (!theNodes || !nodeData || !this->Internal->ASCIIStream.is_open())
    {
        vtkErrorMacro(<< "File not open, errors with reading, or nullptr vtkPoints /"
            << "vtkPointData.");
        return;
    }

    int n, v;
    int zArrayId; // indexing zoneData
    int cordBase; // offset of a 3D-coordinate triple in cordsPtr
    int isXcoord;
    int isYcoord;
    int isZcoord;
    std::vector<float*> pointers;
    std::vector<vtkFloatArray*> zoneData;

    pointers.clear();
    zoneData.clear();

    // geometry: 3D point coordinates (note that this array must be initialized
    // since only 2D coordinates might be provided by a Tecplot file)
    theNodes->SetNumberOfPoints(numNodes);
    auto cords = vtkAOSDataArrayTemplate<float>::FastDownCast(theNodes->GetData());
    cords->FillValue(0);

    // three arrays used to determine the role of each variable (including
    // the coordinate arrays)
    std::vector<int> anyCoord(this->NumberOfVariables); // is any coordinate?
    std::vector<int> coordIdx(this->NumberOfVariables); // index of the coordinate array, just in case
    std::vector<int> selected(this->NumberOfVariables); // is a selected data array?

    // allocate arrays only if necessary to load the zone data
    for (v = 0; v < this->NumberOfVariables; v++)
    {
        isXcoord = int(!(v - this->Internal->XIdInList));
        isYcoord = int(!(v - this->Internal->YIdInList));
        isZcoord = int(!(v - this->Internal->ZIdInList));
        anyCoord[v] = isXcoord + isYcoord + isZcoord;
        coordIdx[v] = isYcoord + (isZcoord << 1);
        selected[v] = this->DataArraySelection->ArrayIsEnabled(this->Variables[v].c_str());

        if (anyCoord[v] + selected[v])
        {
            vtkFloatArray* theArray = vtkFloatArray::New();
            theArray->SetNumberOfTuples(numNodes);
            theArray->SetName(this->Variables[v].c_str());
            zoneData.push_back(theArray);
            float* arrayPtr = theArray->GetPointer(0);
            pointers.push_back(arrayPtr);
            arrayPtr = nullptr;
            theArray = nullptr;
        }
    }

    // load the zone data (number of tuples <= number of points / nodes)
    for (n = 0; n < numNodes; n++)
    {
        cordBase = (n << 1) + n;

        zArrayId = 0;
        for (v = 0; v < this->NumberOfVariables; v++)
        {
            // obtain a value that is either a coordinate or a selected attribute
            if (anyCoord[v] || selected[v])
            {
                float theValue;
                VTK_FROM_CHARS_IF_ERROR_RETURN(this->Internal->GetNextToken(), theValue, );

                pointers[zArrayId++][n] = theValue;

                // collect the coordinate
                if (anyCoord[v])
                {
                    cords->SetValue(cordBase + coordIdx[v], theValue);
                }
            }
            else
            {
                // a value of an un-selected data array
                this->Internal->GetNextToken();
            }
        }
    }

    // attach the node-based data attributes to the grid
    zArrayId = 0;
    for (v = 0; v < this->NumberOfVariables; v++)
    {
        if (!anyCoord[v] && selected[v])
        {
            nodeData->AddArray(zoneData[zArrayId]);
        }

        zArrayId += int(!(!(anyCoord[v] + selected[v])));
    }

    pointers.clear();

    // remove all the float arrays from vector so they won't leak
    for (unsigned int i = 0; i < zoneData.size(); ++i)
    {
        vtkFloatArray* fa = zoneData.at(i);
        if (fa)
        {
            fa->FastDelete();
        }
    }
    zoneData.clear();
}

//------------------------------------------------------------------------------
void vtkTecplotReader::GetArraysFromBlockPackingZone(
    int numNodes, int numCells, vtkPoints* theNodes, vtkPointData* nodeData, vtkCellData* cellData)
{
    // NOTE: The Tecplot ASCII file format states that a block-packing zone may
    // contain point data or cell data (VARLOCATION, pp. 158, Tecplot 360 Data
    // Format Guide 2009). Thus we need to consider both cases in this function.

    if (!theNodes || !nodeData || !cellData || !this->Internal->ASCIIStream.is_open())
    {
        vtkErrorMacro(<< "File not open, errors with reading, or nullptr vtkPoints /"
            << "vtkPointData / vtkCellData.");
        return;
    }

    int v;
    int zArrayId; // indexing zoneData
    int arraySiz;
    int isXcoord;
    int isYcoord;
    int isZcoord;
    std::vector<vtkFloatArray*> zoneData;
    vtkDataSetAttributes* attribute[2] = { nodeData, cellData };

    zoneData.clear();

    // geometry: 3D point coordinates (note that this array must be initialized
    // since only 2D coordinates might be provided by a Tecplot file)
    theNodes->SetNumberOfPoints(numNodes);
    auto cords = vtkAOSDataArrayTemplate<float>::FastDownCast(theNodes->GetData());
    cords->FillValue(0);

    // two arrays used to determine the role of each variable (including
    // the coordinate arrays)
    std::vector<int> anyCoord(this->NumberOfVariables); // is any coordinate?
    std::vector<int> selected(this->NumberOfVariables); // is a selected data array?

    for (v = 0; v < this->NumberOfVariables; v++)
    {
        // check if this variable refers to a coordinate array
        isXcoord = int(!(v - this->Internal->XIdInList));
        isYcoord = int(!(v - this->Internal->YIdInList));
        isZcoord = int(!(v - this->Internal->ZIdInList));
        anyCoord[v] = isXcoord + isYcoord + isZcoord;

        // in case of a data attribute, is it selected by the user?
        selected[v] = this->DataArraySelection->ArrayIsEnabled(this->Variables[v].c_str());

        // obtain the size of the block
        arraySiz = (this->CellBased[v] ? numCells : numNodes);

        if (anyCoord[v] || selected[v])
        {
            // parse the block to extract either coordinates or data attribute
            // values

            // extract the variable array throughout a block
            vtkFloatArray* theArray = vtkFloatArray::New();
            theArray->SetNumberOfTuples(arraySiz);
            theArray->SetName(this->Variables[v].c_str());
            zoneData.push_back(theArray);

            float* arrayPtr = theArray->GetPointer(0);
            for (int i = 0; i < arraySiz; i++)
            {
                VTK_FROM_CHARS_IF_ERROR_RETURN(this->Internal->GetNextToken(), arrayPtr[i], );
            }
            theArray = nullptr;

            // three special arrays are 'combined' to fill the 3D coord array
            if (anyCoord[v])
            {
                float* coordPtr = cords->GetPointer(isYcoord + (isZcoord << 1));
                for (int i = 0; i < arraySiz; i++, coordPtr += 3)
                {
                    *coordPtr = arrayPtr[i];
                }
                coordPtr = nullptr;
            }

            arrayPtr = nullptr;
        }
        else
        {
            // this block contains an un-selected data attribute and we
            // need to read but ignore the values
            for (int i = 0; i < arraySiz; i++)
            {
                this->Internal->GetNextToken();
            }
        }
    }

    // attach the dataset attributes (node-based and cell-based) to the grid
    // NOTE: zoneData[] and this->Variables (and this->CellBased) may differ
    // in the number of the maintained arrays
    zArrayId = 0;
    for (v = 0; v < this->NumberOfVariables; v++)
    {
        if (!anyCoord[v] && selected[v])
        {
            attribute[this->CellBased[v]]->AddArray(zoneData[zArrayId]);
        }

        zArrayId += int(!(!(anyCoord[v] + selected[v])));
    }

    // remove all the float arrays from vector so they won't leak
    for (unsigned int i = 0; i < zoneData.size(); ++i)
    {
        vtkFloatArray* fa = zoneData.at(i);
        if (fa)
        {
            fa->FastDelete();
        }
    }
    zoneData.clear();
    attribute[0] = attribute[1] = nullptr;
}

//------------------------------------------------------------------------------
void vtkTecplotReader::GetStructuredGridFromBlockPackingZone(int iDimSize, int jDimSize,
    int kDimSize, int zoneIndx, const char* zoneName, vtkMultiBlockDataSet* multZone)
{
    if (!zoneName || !multZone)
    {
        vtkErrorMacro("Zone name un-specified or nullptr vtkMultiBlockDataSet.");
        return;
    }

    // determine the topological dimension
    if (jDimSize == 1 && kDimSize == 1)
    {
        this->Internal->TopologyDim = vtkMath::Max(this->Internal->TopologyDim, 1);
    }
    else if (kDimSize == 1)
    {
        this->Internal->TopologyDim = vtkMath::Max(this->Internal->TopologyDim, 2);
    }
    else
    {
        this->Internal->TopologyDim = vtkMath::Max(this->Internal->TopologyDim, 3);
    }

    // number of points, number of cells, and dimensionality
    int numNodes = iDimSize * jDimSize * kDimSize;
    int numCells = ((iDimSize <= 1) ? 1 : (iDimSize - 1)) * ((jDimSize <= 1) ? 1 : (jDimSize - 1)) *
        ((kDimSize <= 1) ? 1 : (kDimSize - 1));
    int gridDims[3] = { iDimSize, jDimSize, kDimSize };

    // Create vtkPoints and vtkStructuredGrid and associate them
    vtkPoints* pntCords = vtkPoints::New();
    vtkStructuredGrid* strcGrid = vtkStructuredGrid::New();
    this->GetArraysFromBlockPackingZone(
        numNodes, numCells, pntCords, strcGrid->GetPointData(), strcGrid->GetCellData());
    strcGrid->SetDimensions(gridDims);
    strcGrid->SetPoints(pntCords);
    pntCords->Delete();
    pntCords = nullptr;

    if ((this->Internal->TopologyDim == 2 || this->Internal->TopologyDim == 3) ||
        ((this->Internal->TopologyDim == 0 || this->Internal->TopologyDim == 1) &&
            this->Internal->GeometryDim > 1))
    {
        multZone->SetBlock(zoneIndx, strcGrid);
        multZone->GetMetaData(zoneIndx)->Set(vtkCompositeDataSet::NAME(), zoneName);
    }
    strcGrid->Delete();
    strcGrid = nullptr;
}

//------------------------------------------------------------------------------
void vtkTecplotReader::GetStructuredGridFromPointPackingZone(int iDimSize, int jDimSize,
    int kDimSize, int zoneIndx, const char* zoneName, vtkMultiBlockDataSet* multZone)
{
    if (!zoneName || !multZone)
    {
        vtkErrorMacro("Zone name un-specified or nullptr vtkMultiBlockDataSet.");
        return;
    }

    if (jDimSize == 1 && kDimSize == 1)
    {
        this->Internal->TopologyDim = vtkMath::Max(this->Internal->TopologyDim, 1);
    }
    else if (kDimSize == 1)
    {
        this->Internal->TopologyDim = vtkMath::Max(this->Internal->TopologyDim, 2);
    }
    else
    {
        this->Internal->TopologyDim = vtkMath::Max(this->Internal->TopologyDim, 3);
    }

    // number of points, number of cells, and dimensionality
    int numNodes = iDimSize * jDimSize * kDimSize;
    int gridDims[3] = { iDimSize, jDimSize, kDimSize };

    // Create vtkPoints and vtkStructuredGrid and associate them
    vtkPoints* pntCords = vtkPoints::New();
    vtkStructuredGrid* strcGrid = vtkStructuredGrid::New();
    this->GetArraysFromPointPackingZone(numNodes, pntCords, strcGrid->GetPointData());
    strcGrid->SetDimensions(gridDims);
    strcGrid->SetPoints(pntCords);
    pntCords->Delete();
    pntCords = nullptr;

    if ((this->Internal->TopologyDim == 2 || this->Internal->TopologyDim == 3) ||
        (this->Internal->TopologyDim == 0 && this->Internal->GeometryDim > 1))
    {
        multZone->SetBlock(zoneIndx, strcGrid);
        multZone->GetMetaData(zoneIndx)->Set(vtkCompositeDataSet::NAME(), zoneName);
    }
    strcGrid->Delete();
    strcGrid = nullptr;
}

void vtkTecplotReader::GetPolygonalGridFromBlockPackingZone(int numNodes, int numCells,
    int numFaces, int zoneIndx, const char* zoneName, vtkMultiBlockDataSet* multZone)
{
    vtkPoints* gridPnts = vtkPoints::New();
    vtkUnstructuredGrid* unstruct = vtkUnstructuredGrid::New();
    this->GetArraysFromBlockPackingZone(
        numNodes, numCells, gridPnts, unstruct->GetPointData(), unstruct->GetCellData());

    unstruct->SetPoints(gridPnts);
    gridPnts->Delete();
    gridPnts = nullptr;

    this->GetPolygonalGridCells(numCells, numFaces, unstruct);

    if ((this->Internal->TopologyDim == 2 || this->Internal->TopologyDim == 3) ||
        (this->Internal->TopologyDim == 0 && this->Internal->GeometryDim > 1))
    {
        multZone->SetBlock(zoneIndx, unstruct);
        multZone->GetMetaData(zoneIndx)->Set(vtkCompositeDataSet::NAME(), zoneName);
    }
    unstruct->Delete();
    unstruct = nullptr;
}

void vtkTecplotReader::GetPolyhedralGridFromBlockPackingZone(int numNodes, int numCells,
    int numFaces, int zoneIndx, const char* zoneName, vtkMultiBlockDataSet* multZone)
{
    vtkPoints* gridPnts = vtkPoints::New();
    vtkUnstructuredGrid* unstruct = vtkUnstructuredGrid::New();
    this->GetArraysFromBlockPackingZone(
        numNodes, numCells, gridPnts, unstruct->GetPointData(), unstruct->GetCellData());

    unstruct->SetPoints(gridPnts);
    gridPnts->Delete();
    gridPnts = nullptr;

    this->GetPolyhedralGridCells(numCells, numFaces, unstruct);

    if ((this->Internal->TopologyDim == 2 || this->Internal->TopologyDim == 3) ||
        (this->Internal->TopologyDim == 0 && this->Internal->GeometryDim > 1))
    {
        multZone->SetBlock(zoneIndx, unstruct);
        multZone->GetMetaData(zoneIndx)->Set(vtkCompositeDataSet::NAME(), zoneName);
    }
    unstruct->Delete();
    unstruct = nullptr;
}

//------------------------------------------------------------------------------
void vtkTecplotReader::GetUnstructuredGridFromBlockPackingZone(int numNodes, int numCells,
    const char* cellType, int zoneIndx, const char* zoneName, vtkMultiBlockDataSet* multZone)
{
    if (!cellType || !zoneName || !multZone)
    {
        vtkErrorMacro(<< "Zone name / cell type un-specified, or nullptr "
            << "vtkMultiBlockDataSet object.");
        return;
    }

    vtkPoints* gridPnts = vtkPoints::New();
    vtkUnstructuredGrid* unstruct = vtkUnstructuredGrid::New();
    this->GetArraysFromBlockPackingZone(numNodes, numCells, gridPnts, unstruct->GetPointData(), unstruct->GetCellData());
    this->GetUnstructuredGridCells(numCells, cellType, unstruct);
    unstruct->SetPoints(gridPnts);
    gridPnts->Delete();
    gridPnts = nullptr;

    if ((this->Internal->TopologyDim == 2 || this->Internal->TopologyDim == 3) ||
        (this->Internal->TopologyDim == 0 && this->Internal->GeometryDim > 1))
    {
        multZone->SetBlock(zoneIndx, unstruct);
        multZone->GetMetaData(zoneIndx)->Set(vtkCompositeDataSet::NAME(), zoneName);
    }
    unstruct->Delete();
    unstruct = nullptr;
}

//------------------------------------------------------------------------------
void vtkTecplotReader::GetUnstructuredGridFromPointPackingZone(int numNodes, int numCells,
    const char* cellType, int zoneIndx, const char* zoneName, vtkMultiBlockDataSet* multZone)
{
    if (!cellType || !zoneName || !multZone)
    {
        vtkErrorMacro(<< "Zone name / cell type un-specified, or nullptr "
            << "vtkMultiBlockDataSet object.");
        return;
    }

    vtkPoints* gridPnts = vtkPoints::New();
    vtkUnstructuredGrid* unstruct = vtkUnstructuredGrid::New();
    this->GetArraysFromPointPackingZone(numNodes, gridPnts, unstruct->GetPointData());
    this->GetUnstructuredGridCells(numCells, cellType, unstruct);
    unstruct->SetPoints(gridPnts);
    gridPnts->Delete();
    gridPnts = nullptr;

    if ((this->Internal->TopologyDim == 2 || this->Internal->TopologyDim == 3) ||
        (this->Internal->TopologyDim == 0 && this->Internal->GeometryDim > 1))
    {
        multZone->SetBlock(zoneIndx, unstruct);
        multZone->GetMetaData(zoneIndx)->Set(vtkCompositeDataSet::NAME(), zoneName);
    }
    unstruct->Delete();
    unstruct = nullptr;
}

void vtkTecplotReader::GetPolyhedralGridCells(
    int numCells, int numFaces, vtkUnstructuredGrid* unstruct) const
{
    auto tok = this->Internal->GetNextToken();
    while (tok.empty())
    {
        tok = this->Internal->GetNextToken();
    }

    std::vector<size_t> nodeCountPerFace;
    size_t count;
    VTK_FROM_CHARS_IF_ERROR_RETURN(tok, count, );
    nodeCountPerFace.push_back(count);

    for (vtkIdType i = 1; i < numFaces; ++i)
    {
        tok = this->Internal->GetNextToken();
        while (tok.empty())
        {
            tok = this->Internal->GetNextToken();
        }
        VTK_FROM_CHARS_IF_ERROR_RETURN(tok, count, );
        nodeCountPerFace.push_back(count);
    }

    std::vector<std::vector<vtkIdType>> faces;
    for (vtkIdType i = 0; i < numFaces; ++i)
    {
        const size_t nodeCount = nodeCountPerFace[i];
        std::vector<vtkIdType> face;
        face.reserve(nodeCount);

        for (size_t j = 0; j < nodeCount; ++j)
        {
            tok = this->Internal->GetNextToken();
            while (tok.empty())
            {
                tok = this->Internal->GetNextToken();
            }
            vtkIdType aVertexIndex;
            VTK_FROM_CHARS_IF_ERROR_RETURN(tok, aVertexIndex, );
            face.push_back(aVertexIndex - 1); // convert from FORTRAN to C-indexing
        }

        faces.push_back(face);
    }

    std::map<vtkIdType, std::vector<vtkIdType>> polyhedra;

    for (vtkIdType i = 0; i < numFaces; ++i)
    {
        tok = this->Internal->GetNextToken();
        while (tok.empty())
        {
            tok = this->Internal->GetNextToken();
        }
        vtkIdType rightCell;
        VTK_FROM_CHARS_IF_ERROR_RETURN(tok, rightCell, );
        if (rightCell > 0)
        {
            polyhedra[rightCell - 1].push_back(i);
        }
    }

    for (vtkIdType i = 0; i < numFaces; ++i)
    {
        tok = this->Internal->GetNextToken();
        while (tok.empty())
        {
            tok = this->Internal->GetNextToken();
        }
        vtkIdType leftCell;
        VTK_FROM_CHARS_IF_ERROR_RETURN(tok, leftCell, );
        if (leftCell > 0)
        {
            polyhedra[leftCell - 1].push_back(i);
        }
    }

    for (auto& entry : polyhedra)
    {
        const auto& facesOfPolyhedron = entry.second;
        std::vector<vtkIdType> polyhedron;

        for (auto& aFaceIndex : facesOfPolyhedron)
        {
            const auto& aFace = faces[aFaceIndex];
            const auto faceSize = static_cast<vtkIdType>(aFace.size());
            polyhedron.push_back(faceSize);
            for (auto& aVertexIndex : aFace)
            {
                polyhedron.push_back(aVertexIndex);
            }
        }
        unstruct->InsertNextCell(
            VTK_POLYHEDRON, static_cast<vtkIdType>(facesOfPolyhedron.size()), polyhedron.data());
    }

    if (unstruct->GetNumberOfCells() != numCells)
    {
        vtkWarningMacro(<< "Number of polyhedral cells does not match.");
    }
}