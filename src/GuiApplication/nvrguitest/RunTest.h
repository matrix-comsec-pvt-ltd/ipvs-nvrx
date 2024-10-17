#ifndef RUNTEST_H
#define RUNTEST_H

#include <QThread>
#include "HardwareTestControl.h"

class RunTest : public QThread
{
    Q_OBJECT
public:
    explicit RunTest(HW_TEST_BLOCK_e testBlock, QWidget *parent = 0);
    ~RunTest();

    void run();
    void enableDisableControls(bool isEnable);
    void changeTestIndicatorState(TEST_RESULT_e testResult);    

    HardwareTestControl*    hwTestControl[MAX_TEST_HW_CONDUCT];
    HW_TEST_BLOCK_e         hwTestBlock;

private:
    CONDUNCT_TEST_e         startTestIndex;
    CONDUNCT_TEST_e         stopTestIndex;

    bool testExecuted(quint8 hwIndex);
    
signals:
    void sigTestCompelete(HW_TEST_BLOCK_e hwTestBlock);
    void sigCtrlBtnClicked(HW_TEST_BLOCK_e hwTestBlock, int hwIndex, int ctrlBtnIndex);
    
public slots:
    void slotShutDown(int value=0);
    void slotCtrlBtnClicked(int hwIndex, int ctrlBtnIndex);
};

#endif // RUNTEST_H
