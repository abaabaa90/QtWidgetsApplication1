//���ڴ��ڻ�������ҪΪ�����ṩһ��ͳһ�Ļ�����ʽ,����˵��������һЩ������������ô��ڵı���ɫ����ͳһ��
#pragma once
#include "pch.h"
#include <QWidget>

class BaseWindow : public QWidget
{
    Q_OBJECT

public:
    explicit BaseWindow(QWidget* parent = nullptr, bool showCloseButton = true);
    virtual ~BaseWindow();

    // ���ô��ڱ���
    void setWindowTitle(const QString& title);

    // ���ô��ڴ�С
    void setWindowSize(int width, int height);
    void setWindowSize(const QSize& size);

    // ���ù̶���С
    void setFixedWindowSize(int width, int height);
    void setFixedWindowSize(const QSize& size);

    // ������С/���ߴ�
    void setMinimumWindowSize(int width, int height);
    void setMaximumWindowSize(int width, int height);

    // ���ñ�����ɫ
    void setBackgroundColor(const QColor& color);

    // ���ô���Բ��
    void setWindowRadius(int radius);

    // ���ô�����Ӱ
    void setWindowShadow(bool enabled, int blurRadius = 10);

    // ���ñ�������ʽ
    void setTitleBarVisible(bool visible);
    void setTitleBarHeight(int height);
    void setTitleBarColor(const QColor& color);

    // ������������߾�
    void setContentMargins(int left, int top, int right, int bottom);

    // ������ʾ���
    void showCentered();  // ������ʾ
    void showMaximized(); // �����ʾ
    void showMinimized(); // ��С����ʾ
    void showNormal();    // ������ʾ

    // ��ȡ��������widget���������������������ݣ�
    QWidget* contentWidget() const;

protected:
    // ���������д��Щ�������Զ�����ʽ
    virtual void initUI();
    virtual void initStyle();
    virtual void initConnections();

    // �¼���д
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    // ������Է��ʵĳ�Ա
    QWidget* m_titleBar;           // ������
    QWidget* m_contentWidget;      // ��������
    QLabel* m_titleLabel;          // �����ǩ
    QPushButton* m_closeButton;    // �رհ�ť
    QPushButton* m_minButton;      // ��С����ť
    QPushButton* m_maxButton;      // ��󻯰�ť

    QVBoxLayout* m_mainLayout;     // ������
    QHBoxLayout* m_titleLayout;    // ����������
    QVBoxLayout* m_contentLayout;  // ���ݲ���

private slots:
    void onCloseClicked();
    void onMinClicked();
    void onMaxClicked();

private:
    // ��ʼ������
    void initMembers();
    void setupLayout();
    void setupTitleBar();
    void setupContentArea();

    // �����϶����
    bool m_isDragging;
    QPoint m_dragStartPosition;

    // ��ʽ���
    QColor m_backgroundColor;
    QColor m_titleBarColor;
    int m_windowRadius;
    bool m_shadowEnabled;
    int m_shadowBlurRadius;

    // ״̬
    bool m_isMaximized;
    QRect m_normalGeometry;

    // ˽�з���
    void updateWindowStyle();
    void drawWindowShadow(QPainter& painter);
    void drawRoundedRect(QPainter& painter, const QRect& rect, int radius);
};