//#include<qmainwindow.h>
//#include <QLineEdit>
//#include <QSpinBox>
//#include <QDoubleSpinBox>
//#include <QListWidget>
//#include <tuple>
//
//// 通用数据类，支持UI控件绑定
//template<typename... Types>
//class DynamicData {
//public:
//    using DataTuple = std::tuple<Types...>;
//
//    // 默认构造函数
//    DynamicData() {}
//
//    // 从UI控件初始化
//    template<typename... Widgets>
//    DynamicData(Widgets*... widgets) {
//        updateFromWidgets(widgets...);
//    }
//
//    // 从值初始化（可选）
//    DynamicData(Types... values) : data(values...) {}
//
//    // 从UI控件更新数据
//    template<typename... Widgets>
//    void updateFromWidgets(Widgets*... widgets) {
//        updateFromWidgetsImpl(widgets..., std::index_sequence_for<Types...>{});
//    }
//
//    // 将数据应用到UI控件
//    template<typename... Widgets>
//    void applyToWidgets(Widgets*... widgets) const {
//        applyToWidgetsImpl(widgets..., std::index_sequence_for<Types...>{});
//    }
//
//    // 获取数据引用
//    DataTuple& getData() { return data; }
//    const DataTuple& getData() const { return data; }
//
//private:
//    DataTuple data;
//
//    // 从UI控件更新数据的实现
//    template<typename... Widgets, size_t... Is>
//    void updateFromWidgetsImpl(Widgets*... widgets, std::index_sequence<Is...>) {
//        ((std::get<Is>(data) = extractValue(widgets)), ...);
//    }
//
//    // 应用到UI控件的实现
//    template<typename... Widgets, size_t... Is>
//    void applyToWidgetsImpl(Widgets*... widgets, std::index_sequence<Is...>) const {
//        ((setValue(widgets, std::get<Is>(data))), ...);
//    }
//
//    // 从不同类型UI控件提取值的模板函数
//    template<typename T>
//    auto extractValue(T* widget) -> decltype(widget->text()) {
//        if constexpr (std::is_same_v<decltype(widget->text()), QString>) {
//            return widget->text();
//        }
//        return widget->text(); // 默认返回text()
//    }
//
//    // 特化处理不同类型的控件
//    int extractValue(QSpinBox* spin) { return spin->value(); }
//    double extractValue(QDoubleSpinBox* spin) { return spin->value(); }
//    QString extractValue(QLineEdit* edit) { return edit->text(); }
//    QList<QString> extractValue(QListWidget* list) {
//        QList<QString> result;
//        for (int i = 0; i < list->count(); ++i) {
//            result.append(list->item(i)->text());
//        }
//        return result;
//    }
//
//    // 设置不同类型的控件值
//    void setValue(QSpinBox* spin, int value) { spin->setValue(value); }
//    void setValue(QDoubleSpinBox* spin, double value) { spin->setValue(value); }
//    void setValue(QLineEdit* edit, const QString& value) { edit->setText(value); }
//    void setValue(QListWidget* list, const QList<QString>& values) {
//        list->clear();
//        for (const auto& value : values) {
//            list->addItem(value);
//        }
//    }
//};
//
//// 序列化器（保持通用性）
//template<typename T>
//class Serializer {
//public:
//    static bool save(const T& obj, const QString& filename) {
//        QFile file(filename);
//        if (!file.open(QIODevice::WriteOnly)) return false;
//        QDataStream out(&file);
//        out.setVersion(QDataStream::Qt_5_14);
//        saveTuple(out, obj.getData(), std::make_index_sequence<std::tuple_size_v<decltype(obj.getData())>>{});
//        return true;
//    }
//
//    static bool load(T& obj, const QString& filename) {
//        QFile file(filename);
//        if (!file.open(QIODevice::ReadOnly)) return false;
//        QDataStream in(&file);
//        in.setVersion(QDataStream::Qt_5_14);
//        loadTuple(in, obj.getData(), std::make_index_sequence<std::tuple_size_v<decltype(obj.getData())>>{});
//        return true;
//    }
//
//private:
//    template<typename Stream, typename Tuple, size_t... Is>
//    static void saveTuple(Stream& out, const Tuple& tuple, std::index_sequence<Is...>) {
//        ((out << std::get<Is>(tuple)), ...);
//    }
//
//    template<typename Stream, typename Tuple, size_t... Is>
//    static void loadTuple(Stream& in, Tuple& tuple, std::index_sequence<Is...>) {
//        ((in >> std::get<Is>(tuple)), ...);
//    }
//};
//// 在你的窗口类中使用
//class MainWindow : public QMainWindow {
//    Q_OBJECT
//
//public:
//    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
//        
//
//        // 定义数据类型（自动推导）
//        using MyDataType = DynamicData<QString, int, double, QList<QString>>;
//
//        // 从UI控件创建数据对象
//        data = MyDataType(nameEdit, ageSpin, scoreSpin, tagsList);
//    }
//
//    // 保存按钮点击
//    void onSaveClicked() {
//        // 更新数据（从UI控件获取当前值）
//        data.updateFromWidgets(nameEdit, ageSpin, scoreSpin, tagsList);
//
//        // 保存（一行代码）
//        Serializer<decltype(data)>::save(data, "userdata.dat");
//    }
//
//    // 加载按钮点击
//    void onLoadClicked() {
//        // 加载（一行代码）
//        if (Serializer<decltype(data)>::load(data, "userdata.dat")) {
//            // 将数据应用到UI控件
//            data.applyToWidgets(nameEdit, ageSpin, scoreSpin, tagsList);
//        }
//    }
//
//private:
//    QLineEdit* nameEdit;
//    QSpinBox* ageSpin;
//    QDoubleSpinBox* scoreSpin;
//    QListWidget* tagsList;
//
//    // 自动推导类型
//    decltype(DynamicData(nameEdit, ageSpin, scoreSpin, tagsList)) data;
//};