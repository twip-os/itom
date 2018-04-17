#include "foldDetector.h"

#include "codeEditor.h"
#include <qpointer.h>
#include "utils/utils.h"

class FoldDetectorPrivate
{
public:
    FoldDetectorPrivate() :
        m_limit(std::numeric_limits<int>::max())
    {
        /*
        #: Reference to the parent editor, automatically set by the syntax
        #: highlighter before process any block.

        Fold level limit, any level greater or equal is skipped.
        #: Default is sys.maxsize (i.e. all levels are accepted)*/
    }

    QPointer<CodeEditor> m_editor;
    int m_limit;
};

//--------------------------------------------------
FoldDetector::FoldDetector(QObject *parent /*= NULL*/) :
    QObject(parent),
    d_ptr(new FoldDetectorPrivate)
{
}

//--------------------------------------------------
FoldDetector::~FoldDetector()
{
    delete d_ptr;
    d_ptr = NULL;
}

//--------------------------------------------------
/*
Processes a block and setup its folding info.

This method call ``detect_fold_level`` and handles most of the tricky
corner cases so that all you have to do is focus on getting the proper
fold level foreach meaningful block, skipping the blank ones.

:param current_block: current block to process
:param previous_block: previous block
:param text: current block text
*/
void FoldDetector::processBlock(QTextBlock &currentBlock, QTextBlock &previousBlock, const QString &text)
{
    Q_D(FoldDetector);

    int prev_fold_level = Utils::TextBlockHelper::getFoldLvl(previousBlock);
    int fold_level;
    if (text.trimmed() == "")
    {
        // blank line always have the same level as the previous line
        fold_level = prev_fold_level;
    }
    else
    {
        fold_level = detectFoldLevel(previousBlock, currentBlock);
        if (fold_level > d->m_limit)
        {
            fold_level = d->m_limit;
        }
    }

    prev_fold_level = Utils::TextBlockHelper::getFoldLvl(previousBlock);

    if (fold_level > prev_fold_level)
    {
        // apply on previous blank lines
        QTextBlock block = currentBlock.previous();
        while (block.isValid() && block.text().trimmed() == "")
        {
            Utils::TextBlockHelper::setFoldLvl(block, fold_level);
            block = block.previous();
        }
        Utils::TextBlockHelper::setFoldTrigger(block, true);
    }

    // update block fold level
    if (text.trimmed() != "")
    {
        Utils::TextBlockHelper::setFoldTrigger(previousBlock, fold_level > prev_fold_level);
    }
    Utils::TextBlockHelper::setFoldLvl(currentBlock, fold_level);

    // user pressed enter at the beginning of a fold trigger line
    // the previous blank line will keep the trigger state and the new line
    // (which actually contains the trigger) must use the prev state (
    // and prev state must then be reset).
    QTextBlock prev = currentBlock.previous();  // real prev block (may be blank)
    if (prev.isValid() && prev.text().trimmed() == "" & Utils::TextBlockHelper::isFoldTrigger(prev))
    {
        // prev line has the correct trigger fold state
        Utils::TextBlockHelper::setCollapsed(currentBlock, Utils::TextBlockHelper::isCollapsed(prev));
        // make empty line not a trigger
        Utils::TextBlockHelper::setFoldTrigger(prev, false);
        Utils::TextBlockHelper::setCollapsed(prev, false);
    }
}

//--------------------------------------------------
CodeEditor* FoldDetector::editor() const
{
    Q_D(const FoldDetector);
    return d->m_editor;
}

//--------------------------------------------------
void FoldDetector::setEditor(CodeEditor *editor)
{
    Q_D(FoldDetector);
    d->m_editor = QPointer<CodeEditor>(editor);
}