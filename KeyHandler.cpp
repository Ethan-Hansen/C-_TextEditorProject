#include "ECTextViewImp.h"
#include "ECTextDocument.h"
#include "ECObserver.h"
#include <vector>
#include <iostream>
#include <string>

/* ********************************************************************
    Chain of Command Design pattern
    Each handler corresponds to a keypress.
    The main Key Handler object passes the key press
    down the chain of handlers until one satisfies it,
    then that handler either does the function or creates
    the command and executes it using ECCommandHistory::ExecuteCommand()
******************************************************************** */
void InsertHandler :: Handle(int key)
{
    //Checks ASCII values
    if(key > 31 && key < 127)
    {
       int offset = (*visible).GetOffset();
       int para = (*visible).GetRow();

       char x = key;

        InsertCommand* com = new InsertCommand(visible, doc, (*visible).GetParagraph(para), x, offset, para);
       (*comHist).ExecuteCommand(com);

       (*visible).Update();
    }
    else
    {
        KeyHandler::Handle(key);
    }
}

void CursorHandler :: Handle(int key)
{
    int row = (*doc).GetCursorY();
    int pos = (*doc).GetCursorX();
    int currpage = (*visible).GetPage();

    if(key == ARROW_RIGHT)
    {
        //If not at end of line
        if(pos < (*visible).GetDisplayLineSize(row))
        {
            (*doc).SetCursorX(pos + 1);
        }
        //else if at end of line
        else if (((row + 1) + ((*doc).GetRowNumInView() - 1) * currpage) < (*visible).GetDisplaySize())
        {
            if(row != (*doc).GetRowNumInView() - 2)
            {
                (*doc).SetCursorX(0);
                (*doc).SetCursorY(row + 1);
            }
            else
            {
                (*doc).SetCursorY(0);
                (*doc).SetCursorX(0);
                (*visible).SetPage(currpage + 1);
                (*visible).Update();
            }
        }
        
    }
    else if (key == ARROW_LEFT)
    {
        //If not at end of line
        if(pos != 0)
        {
            (*doc).SetCursorX(pos - 1);
        }
        //Else at end of line
        else
        {
            int prev = (*visible).PreviousRowSize();
            //Checks to make sure not at last row
            if(row != 0)
            {
                (*doc).SetCursorX(prev);
                (*doc).SetCursorY(row - 1);
            }
            //If at last row but not last page
            if(row == 0 && currpage != 0)
            {
                (*visible).SetPage(currpage - 1);
                (*doc).SetCursorY((*doc).GetRowNumInView() - 2);
                (*doc).SetCursorX(prev);
                (*visible).Update();
            }
        }
        
    }
    else if (key == ARROW_UP)
    {
        //If not at top of document
        if(row != 0)
        {
            int size = (*visible).PreviousRowSize();
            //If current line is bigger then above line
            if(pos > size)
            {
                (*doc).SetCursorX(size);
            }
            (*doc).SetCursorY(row - 1);   
        }
        else
        {
            //If at top of page
            if(currpage != 0)
            {
                int size = (*visible).PreviousRowSize();
                //If current line is bigger than above line
                if(pos > size)
                {
                    (*doc).SetCursorX(size);
                }
                (*doc).SetCursorY((*doc).GetRowNumInView() - 2);
                (*visible).SetPage(currpage - 1);
                (*visible).Update();
            }
        }
        
    }
    else if (key == ARROW_DOWN)
    {
        //If not at bottom of document
        if(row + 1 < (*visible).GetDisplaySize() - (((*doc).GetRowNumInView() - 1) * currpage))
        {
            //If at bottom of page
            if(row == (*doc).GetRowNumInView() - 2)
            {
                int size = (*visible).NextRowSize();
                //If currentline is larger than line below
                if(pos > size)
                {
                    (*doc).SetCursorX(size);
                }
                (*doc).SetCursorY(0);
                (*visible).SetPage(currpage + 1);
                (*visible).Update();                 
            }
            //Else not at bottom of page
            else
            {
                int size = (*visible).NextRowSize();
                //If currentline is larger than line below
                if(pos > size)
                {
                    (*doc).SetCursorX(size);
                }
                (*doc).SetCursorY(row + 1);   
            }
        }
    }
    else
    {
        KeyHandler::Handle(key);
    }
}

void RemoveHandler :: Handle(int key)
{
    if(key == BACKSPACE)
    {
        //If not at top of doc
        if(!((*doc).GetCursorX() == 0 && (*doc).GetCursorY() == 0 && (*visible).GetPage() == 0))
        {
            int offset = (*visible).GetOffset();
            int para = (*visible).GetRow();

            RemoveCommand* com = new RemoveCommand(visible, doc, (*visible).GetParagraph(para), offset, para);
            (*comHist).ExecuteCommand(com);

            (*visible).Update();
        }
    }
    else
    {
        KeyHandler::Handle(key);
    }
}

void RedoHandler :: Handle(int key)
{
    if(key == CTRL_Y)
    {
        (*comHist).Redo();
        (*visible).Update();
    }
    else
    {
        KeyHandler::Handle(key);
    }
}

void UndoHandler :: Handle(int key)
{
    if(key == CTRL_Z)
    {
        (*comHist).Undo();
        (*visible).Update();
    }
    else
    {
        KeyHandler::Handle(key);
    }
}

void NewLineHandler :: Handle(int key)
{
    if(key == ENTER)
    {
        int offset = (*visible).GetOffset();
        int para = (*visible).GetRow();

        NewLineCommand* com = new NewLineCommand(doc, (*visible).GetParagraph(para), visible, offset, para);
        (*comHist).ExecuteCommand(com);

        (*visible).Update();
    }
    else
    {
        (*this).KeyHandler::Handle(key);
    }
}

void QuitHandler :: Handle(int key)
{
    if(key == CTRL_Q)
    {
        (*visible).WriteFile();
        (*doc).Quit();
        (*doc).InitRows();
    }
    else
    {
        KeyHandler::Handle(key);            
    }
}