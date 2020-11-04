/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     xiajing <xiajing@uniontech.com>
*
* Maintainer: jingzhou <jingzhou@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "memhiswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "dthememanager.h"
#include "utils.h"
#include "math/quantity.h"

MemHisWidget::MemHisWidget(QWidget *parent)
    : DWidget(parent)
    , m_stackWidget(new QStackedWidget)
    , m_listView(new SimpleListView(1))
    , m_listDelegate(new SimpleListDelegate(1, this))
    , m_listModel(new SimpleListModel(1, this))
    , m_buttonBox(new DButtonBox(this))
    , m_memoryBtn(new DButtonBoxButton(QIcon(), {}, this))
, m_historyBtn(new DButtonBoxButton(QIcon(), {}, this))
, m_clearButton(new IconButton(this, 1))
{
    m_memoryPublic = MemoryPublic::instance(this);
    m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::scientificright);
    m_memoryWidget->setFocusPolicy(Qt::TabFocus);
    m_stackWidget->addWidget(m_memoryWidget);
    m_listView->setModel(m_listModel);
    m_listView->setItemDelegate(m_listDelegate);
    m_stackWidget->addWidget(m_listView);
    m_stackWidget->setCurrentWidget(m_memoryWidget);
    m_stackWidget->setFixedSize(451, 302);
    m_buttonBox->setFixedSize(60, 25);
    m_buttonBox->setFocusPolicy(Qt::NoFocus);
    m_memoryBtn->setObjectName("mButtonBoxButton");
    m_memoryBtn->setFixedSize(30, 25);
    m_memoryBtn->setIcon(QIcon(":/assets/images/icon_memory_checked.svg"));
    m_memoryBtn->setIconSize(QSize(30, 25));
    m_historyBtn->setFixedSize(30, 25);
    m_historyBtn->setIcon(QIcon(":/assets/images/icon_history_normal.svg"));
    m_historyBtn->setIconSize(QSize(30, 25));
    m_memoryBtn->setFocusPolicy(Qt::TabFocus);
    QList<DButtonBoxButton *> listBtnBox;
    listBtnBox << m_memoryBtn << m_historyBtn;
    m_buttonBox->setButtonList(listBtnBox, true);
    m_buttonBox->setId(m_memoryBtn, 0);
    m_buttonBox->setId(m_historyBtn, 1);
    m_buttonBox->button(0)->setChecked(true);

    QVBoxLayout *m_Vlayout = new QVBoxLayout(this);
    QWidget *hwidget = new QWidget(this);
    QHBoxLayout *m_Hlayout = new QHBoxLayout();
    m_Hlayout->addSpacing(12);
    m_Hlayout->addWidget(m_buttonBox);
    m_Hlayout->addSpacing(333);
    QWidget *clearwidget = new QWidget(this);
    clearwidget->setFixedSize(36, 36);
    m_clearButton->setParent(clearwidget);
    m_clearButton->setObjectName("clearbtn");
    m_clearButton->showtooltip(true); //设置内存垃圾桶tooltip
    if (m_memoryWidget->isWidgetEmpty())  //防止在其他模式下初始化有内存切换至科学模式清除按钮隐藏
        m_clearButton->setHidden(true);
    m_Hlayout->addWidget(clearwidget);
    m_Hlayout->addSpacing(10);
    m_Hlayout->setMargin(0);
    hwidget->setLayout(m_Hlayout);
    hwidget->setFixedHeight(48);
    m_Vlayout->addWidget(m_stackWidget);
    m_Vlayout->addWidget(hwidget);
    m_Vlayout->setSpacing(0);
    m_Vlayout->setMargin(0);
    m_Vlayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_Vlayout);
    this->setContentsMargins(0, 0, 0, 0);

    m_historyBtn->installEventFilter(this);
    m_memoryBtn->installEventFilter(this);
    m_memoryWidget->findChild<MemoryListWidget *>()->installEventFilter(this);
    m_listView->installEventFilter(this);
    m_clearButton->installEventFilter(this);
    m_stackWidget->installEventFilter(this);
    this->installEventFilter(this);

    setTabOrder(m_stackWidget, m_memoryBtn);
    setTabOrder(m_memoryBtn, m_clearButton);

    //信号槽
    connect(m_buttonBox->button(0), &QAbstractButton::clicked, this, [ = ]() {
        if (!m_buttonBox->button(0)->hasFocus() && QApplication::focusWidget() != nullptr)
            this->setFocus();
        m_clearButton->showtooltip(true); //设置历史垃圾桶tooltip
        m_stackWidget->setCurrentWidget(m_memoryWidget);
        m_clearButton->setHidden(!m_isshowM);

        m_memoryBtn->setIcon(QIcon(":/assets/images/icon_memory_checked.svg"));
        m_historyBtn->setIcon(QIcon(":/assets/images/icon_history_normal.svg"));
    });
    connect(m_buttonBox->button(1), &QAbstractButton::clicked, this, [ = ]() {
        if (!m_buttonBox->button(1)->hasFocus() && QApplication::focusWidget() != nullptr)
            this->setFocus();
        m_clearButton->showtooltip(false); //设置内存垃圾桶tooltip
        m_stackWidget->setCurrentWidget(m_listView);
        m_clearButton->setHidden(!m_isshowH);

        m_memoryBtn->setIcon(QIcon(":/assets/images/icon_memory_normal.svg"));
        m_historyBtn->setIcon(QIcon(":/assets/images/icon_history_checked.svg"));
    });
    connect(m_clearButton, &IconButton::clicked, this, [ = ]() {
        this->setFocus(); //让下次焦点在m_memoryBtn
        if (m_stackWidget->currentWidget() != m_memoryWidget) {
            m_listModel->clearItems();
            emit hisIsFilled(false);
            m_listView->listItemFill(false);
            m_isshowH = false;
            m_listView->setFocusPolicy(Qt::NoFocus);
        } else {
            m_memoryPublic->memoryclean();
            m_isshowM = false;
            m_memoryWidget->setFocusPolicy(Qt::NoFocus);
        }
        m_clearButton->setHidden(true);
    });
    connect(m_clearButton, &TextButton::space, this, [ = ]() { //清除焦点空格事件
        this->setFocus(); //让下次焦点在m_memoryBtn
        if (m_stackWidget->currentWidget() != m_memoryWidget) {
            m_listModel->clearItems();
            emit hisIsFilled(false);
            m_listView->listItemFill(false);
            m_isshowH = false;
            m_listView->setFocusPolicy(Qt::NoFocus);
        } else {
            m_memoryPublic->memoryclean();
            m_isshowM = false;
            m_memoryWidget->setFocusPolicy(Qt::NoFocus);
        }
        m_clearButton->setHidden(true);
    }); //focus下空格按下
    connect(m_listModel, &SimpleListModel::hisbtnhidden, this, [ = ]() {
        m_listModel->clearItems(); //历史记录无数据信号接收
        m_listView->setFocusPolicy(Qt::NoFocus);
        m_listView->listItemFill(false);
        m_isshowH = false;
        emit hisIsFilled(false);
        if (m_stackWidget->currentWidget() != m_memoryWidget)
            m_clearButton->setHidden(true);
    });
    connect(m_memoryPublic, &MemoryPublic::filledMem, this, [ = ]() {
        m_isshowM = true; //公共内存中有数据信号接收
        m_memoryWidget->setFocusPolicy(Qt::TabFocus);
        if (m_stackWidget->currentWidget() == m_memoryWidget)
            m_clearButton->setHidden(false);
    });
    connect(m_memoryPublic, &MemoryPublic::emptyMem, this, [ = ]() {
        m_isshowM = false; //公共内存中无数据信号接收
        m_memoryWidget->setFocusPolicy(Qt::NoFocus);
        if (m_stackWidget->currentWidget() == m_memoryWidget)
            m_clearButton->setHidden(true);
    });
}

MemHisWidget::~MemHisWidget()
{

}

/**
 * @brief 现实历史记录侧时调用，设置buttonbox状态
 */
void MemHisWidget::focusOnButtonbox(Qt::FocusReason Reason)
{
    if (m_stackWidget->currentWidget() == m_memoryWidget) {
        m_buttonBox->button(0)->setChecked(true);
    } else {
        m_buttonBox->button(1)->setChecked(true);
    }
    this->setFocus(Reason);
}

/**
 * @brief 对公共内存进行操作
 */
void MemHisWidget::memoryFunctions(MemHisWidget::memOperate operate, Quantity answer, int row)
{
    switch (operate) {
    case memoryplus:
        m_memoryPublic->memoryplus(answer);
        break;
    case memoryminus:
        m_memoryPublic->memoryminus(answer);
        break;
    case memoryclean:
        m_memoryPublic->memoryclean();
        break;
    case widgetplus:
        m_memoryPublic->widgetplus(row, answer);
        break;
    case widgetminus:
        m_memoryPublic->widgetminus(row, answer);
        break;
    default:
        m_memoryPublic->generateData(answer);
        break;
    }
}

void MemHisWidget::resetFocus()
{
    m_isshowH ? m_listView->setFocusPolicy(Qt::TabFocus) : m_listView->setFocusPolicy(Qt::NoFocus);
    m_isshowM ? m_memoryWidget->setFocusPolicy(Qt::TabFocus) : m_memoryWidget->setFocusPolicy(Qt::NoFocus);
}

void MemHisWidget::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}

/**
 * @brief 左右切换buttonbox焦点
 */
void MemHisWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Left && (m_historyBtn->hasFocus() ||  m_memoryBtn->hasFocus())) {
//        focusNextChild();//焦点移动
        m_memoryBtn->setFocus();
    } else if (e->key() == Qt::Key_Right && (m_historyBtn->hasFocus() ||  m_memoryBtn->hasFocus())) {
//        focusNextChild();//焦点移动
        m_historyBtn->setFocus();
    } else if (e->key() == Qt::Key_Escape) {
        emit hideWidget();
    }
}

/**
 * @brief tab切焦点保证焦点在m_historyBtn
 * @param obj m_memoryBtn
 */
bool MemHisWidget::eventFilter(QObject *obj, QEvent *event)
{
    //防止tab到m_historybtn
    if (obj == m_historyBtn && event->type() == QEvent::FocusIn) {
        QFocusEvent *focus_Event = static_cast<QFocusEvent *>(event);
        if (focus_Event->reason() == Qt::TabFocusReason) {
            m_memoryBtn->setFocus();
            return true;
        }
    }

    if (obj == this) {
        if (event->type() == QEvent::KeyPress) {
            //焦点在该widget上点击tab切换到stackwidget
            QKeyEvent *key_event = static_cast <QKeyEvent *>(event);
            if (key_event->key() == Qt::Key_Tab && !m_clearButton->isHidden()) {
                m_stackWidget->currentWidget()->setFocus(Qt::TabFocusReason);
                return true;
            }
        } else if (event->type() == QEvent::FocusOut) {
            //点击标题栏及外部桌面失去焦点时切换至scientifickeypad
            QFocusEvent *focus_Event = static_cast<QFocusEvent *>(event);
            if (focus_Event->reason() == Qt::MouseFocusReason || focus_Event->reason() == Qt::ActiveWindowFocusReason) {
                emit hideWidget();
            }
        }
    }

    if (obj == m_memoryWidget->findChild<MemoryListWidget *>() || obj == m_clearButton || obj == m_listView
            || obj == m_memoryBtn || obj == m_historyBtn) {
        if (event->type() == QEvent::KeyPress) {
            //焦点循环
            QKeyEvent *key_event = static_cast < QKeyEvent *>(event); //将事件转化为键盘事件
            if (key_event->key() == Qt::Key_Tab) {
                if (m_memoryWidget->findChild<MemoryListWidget *>()->hasFocus()) {
//                    focusNextChild();//焦点移动
                    m_memoryBtn->setFocus(Qt::TabFocusReason);
                } else if (m_listView->hasFocus()) {
//                    focusNextChild();//焦点移动
                    m_memoryBtn->setFocus(Qt::TabFocusReason);
                } else if ((m_memoryBtn->hasFocus() || m_historyBtn->hasFocus()) && !m_clearButton->isHidden()) {
//                    focusNextChild();//焦点移动
                    m_clearButton->setFocus(Qt::TabFocusReason);
                } else if (m_clearButton->hasFocus()) {
//                    focusNextChild();//焦点移动
                    if (m_stackWidget->currentWidget() == m_memoryWidget)
//                        m_memoryWidget->findChild<MemoryListWidget *>()->setFocus();
                        m_memoryWidget->setFocus(Qt::TabFocusReason);
                    else
                        m_listView->setFocus(Qt::TabFocusReason);
                }
                return true; //在该对象点击tab不让焦点到窗口外
            } else if (key_event->key() == Qt::Key_Escape) {
                emit hideWidget();
            }
        } else if (event->type() == QEvent::FocusOut) {
            //点击标题栏及外部桌面失去焦点时切换至scientifickeypad
            QFocusEvent *focus_Event = static_cast<QFocusEvent *>(event);
            if (focus_Event->reason() == Qt::MouseFocusReason || focus_Event->reason() == Qt::ActiveWindowFocusReason) {
                emit hideWidget();
            }
        }

    }
    return QWidget::eventFilter(obj, event);
}

void MemHisWidget::focusInEvent(QFocusEvent *event)
{
    if (event->reason() == Qt::TabFocusReason) {
        if (m_stackWidget->currentWidget() == m_memoryWidget) {
            m_memoryWidget->setFocus(Qt::TabFocusReason);
        } else {
            m_listView->setFocus(Qt::TabFocusReason);
        }
    }
    QWidget::focusInEvent(event);
}

/**
 * @brief 根据主题变换更换垃圾桶切图
 */
void MemHisWidget::themeChanged(int type)
{
    QString path;
    int typeIn = type;
    if (typeIn == 0) {
        typeIn = DGuiApplicationHelper::instance()->themeType();
    }
    if (typeIn == 2)
        path = QString(":/assets/images/%1/").arg("dark");
    else
        path = QString(":/assets/images/%1/").arg("light");
    m_clearButton->setIconUrl(path + "empty_normal.svg", path + "empty_hover.svg", path + "empty_press.svg", 1);
    m_memoryPublic->setThemeType(typeIn);
    m_listDelegate->setThemeType(typeIn);
}

/**
 * @brief 历史记录中有内容tab
 */
void MemHisWidget::historyfilled()
{
    if (m_isshowH == false)
        m_listModel->deleteItem(1);
    m_listView->listItemFill(true);
    m_listView->setFocusPolicy(Qt::TabFocus);
    m_isshowH = true;
    emit hisIsFilled(true);
    if (m_stackWidget->currentWidget() != m_memoryWidget)
        m_clearButton->setHidden(false);
}