#include "ECTextViewImp.h"
#include "ECTextDocument.h"
#include "ECObserver.h"
#include <vector>
#include <iostream>
#include <string>
/* ***************************
    COMMAND METHODS
    There is a command corresponding to each
    editing feature that requires redo/undo functionality.
    Each command is initialized within a handler.
    Each command has access to the ECTextViewImp, the ECVisible,
    and the Rows object that it is dealing with
*************************** */
void InsertCommand :: execute()
{
    //Set boolean is for redo to unsure op is redone in correct position
    if(set == false)
    {
        ppos = (*doc).GetCursorX();
        prow = (*doc).GetCursorY();
        ppage = (*visible).GetPage();
        set = true;
    }
    else
    {
        (*doc).SetCursorX(ppos);
        (*doc).SetCursorY(prow);
        (*visible).SetPage(ppage);
    }

    //For redo
    (*doc).SetCursorY(prow);

    //Makes sure that the line being worked on corresponds to position in page
    rowlist = (*visible).GetParagraph(current_paragraph);

    //Tracks beginning and end of current word within the string before insertion
    int old_word_end = (*rowlist).GetNextSpace(ppos + offset);
    int old_word_start = (*rowlist).GetLastSpace(ppos + offset);

    //Inserts character into position in string
    (*rowlist).Add(ins, ppos + offset);

    //Tracks beginning and end of current word after insertion
    int word_end = (*rowlist).GetNextSpace(ppos + offset);
    int word_start = (*rowlist).GetLastSpace(ppos + offset);

    frow = prow;
    
    //If inserting at the end of a line
    if((*doc).GetCursorX() > (*doc).GetColNumInView() - 1)
    {
        frow = frow + 1;
        //If at end of page
        if(frow > (*doc).GetRowNumInView() - 2)
        {
            (*visible).SetPage(ppage + 1);
            (*doc).SetCursorX(ppos - (word_start - offset));
            (*doc).SetCursorY(0);
        }
        else
        {
            (*doc).SetCursorY(frow);
            (*doc).SetCursorX((ppos + offset) - word_start);
        }
    }
    //Else if word gets moved down a line 
    else if(word_end - offset > (*doc).GetColNumInView() - 1)
    {
        frow = frow + 1;
        //If at the end of a page
        if(frow > (*doc).GetRowNumInView() - 2)
        {
            (*visible).SetPage(ppage + 1);
            (*doc).SetCursorX((ppos + offset) - word_start);
            (*doc).SetCursorY(0);
        }
        else
        {
            (*doc).SetCursorY((*doc).GetCursorY() + 1);
            (*doc).SetCursorX((ppos + offset) - word_start);
        }
    }
    //Else if a space was inserted
    else if(ins == ' ')
    {
        if(word_start < offset)
        {
            if((word_end - word_start - 1) + (*visible).PreviousRowSize() < (*doc).GetColNumInView())
            {
                (*doc).SetCursorX(0);
            }
            else
            {
                (*doc).SetCursorX(ppos + 1);
            }
            
        }
        else
        {
            (*doc).SetCursorX(ppos + 1);
        }
        
    }
    else
    {
        (*doc).SetCursorX(ppos + 1);
    }
}
void InsertCommand :: unexecute()
{
    //Assures line being dealt with is correct line
    rowlist = (*visible).GetParagraph(current_paragraph);

    //Removes inserted character
    (*rowlist).Delete(ppos + offset);

    //Resets cursor
    (*doc).SetCursorY(prow);
    (*doc).SetCursorX(ppos);
    (*visible).SetPage(ppage);
}

void RemoveCommand :: execute()
{
    //For undo/redo to assure correct position and page
    if(set == false)
    {
        ppos = (*doc).GetCursorX();
        prow = (*doc).GetCursorY();
        ppage = (*visible).GetPage();
        set = true;
    }
    else
    {
        (*doc).SetCursorX(ppos);
        (*doc).SetCursorY(prow);
        (*visible).SetPage(ppage);
    }
    
    //Assures current line being worked on is correct
    int remvprev = (*visible).PreviousRowSize();
    rowlist = (*visible).GetParagraph(current_paragraph);
    int word_start = (*rowlist).GetLastSpace(ppos + offset);
    int word_end = (*rowlist).GetNextSpace(ppos + offset);

    //If cursor is currently at start of a paragraph
    if(ppos == 0 && offset == 0)
    {
        (*doc).SetCursorX((*visible).PreviousRowSize());
        
        //Lostline = Deleted paragraph
        lostline = (*rowlist).GetLine();

        //Deletes paragraph
        int current_paragraph = (*visible).GetRow();
        (*visible).Delete(current_paragraph);

        //Gets the previous paragraph
        rowlist = (*visible).GetParagraph(current_paragraph - 1);

        //Appends deleted paragraph to previous one
        int tempsize = (*rowlist).GetLine().size();
        for(int i = 0; i < lostline.size(); i++)
        {
            (*rowlist).Add(lostline[i], tempsize + i);
        }

        //Checks for end of page
        if(prow == 0 && ppage != 0)
        {
            (*doc).SetCursorX((*visible).PreviousRowSize());
            (*doc).SetCursorY((*doc).GetRowNumInView() - 2);
            (*visible).SetPage(ppage - 1);
        }
        else
        {
            //Start of paragraph is going up a row
            if(remvprev + (word_end - word_start) < (*doc).GetColNumInView() - 2)
            {
                (*doc).SetCursorY(prow - 1);
            }
            else
            {
                std::string temp = (*rowlist).GetLine();
                int new_start = (*rowlist).GetLastSpace(temp.size() - lostline.size());
                int new_end = (*rowlist).GetNextSpace(temp.size() - lostline.size());
                (*doc).SetCursorX((new_end - new_start) - (word_end - word_start) - 1);
            }
            
        }
        
        deleted_line = true;
    }
    else
    {
        remv = (*rowlist).GetLine()[ppos + offset - 1];

        //In this case, the current word_start/end assignements would be invalid
        if((ppos + offset) == (*rowlist).GetLine().size())
        {
            word_start = (*rowlist).GetLastSpace(ppos + offset - 1);
            word_end = (*rowlist).GetNextSpace(ppos + offset - 1);
        }

        (*rowlist).Delete(ppos + offset - 1);
        int new_word_start = (*rowlist).GetLastSpace(ppos + offset - 1);
        int new_word_end = (*rowlist).GetNextSpace(ppos + offset - 1);

        //If word was one line up
        if(word_start < offset)
        {
            int prev = (*visible).PreviousRowSize();
            //If appending to a word that is now to long to stay on the line above
            //(word coming down a line)
            if((remvprev + ((word_end - word_start) - 2)) < (*doc).GetColNumInView())
            {
                int newx = (word_end - word_start) - ppos;
                
                //Page check
                if(prow == 0 && ppage != 0)
                {
                    (*doc).SetCursorX(((*doc).GetColNumInView() - newx)); 
                    (*visible).SetPage(ppage - 1);
                    (*doc).SetCursorY((*doc).GetRowNumInView() - 2);
                }
                else
                {
                        (*doc).SetCursorY(prow - 1);
                        (*doc).SetCursorX(((*doc).GetColNumInView() - newx)); 
                }
            }
            else
            {
                //Keeps cursor from going out of bounds
                if(ppos != 0)
                {
                    (*doc).SetCursorX(ppos - 1);
                }
                //Sets cursor to wear the words were joined
                else
                {
                    (*doc).SetCursorX((new_word_end - new_word_start) - (word_end - word_start));
                }
                
            }   
        }
        //else if appended word is too big for the line
        else if((new_word_end - offset) > (*doc).GetColNumInView() - 1)
        {
            if(prow + 1 > (*doc).GetRowNumInView() - 2)
            {
                (*visible).SetPage(ppage + 1);
                (*doc).SetCursorY(0);
            }
            else
            {
                (*doc).SetCursorY(prow + 1);
            }
            
            (*doc).SetCursorX(ppos - (new_word_start - offset) - 2);
        }
        //Else normal remove
        else
        {
            (*doc).SetCursorX(ppos - 1);
        }
        
    }
    
}
void RemoveCommand :: unexecute()
{
    (*doc).SetCursorX(ppos);
    (*doc).SetCursorY(prow);
    (*visible).SetPage(ppage);
    rowlist = (*visible).GetParagraph(current_paragraph);

    //If a paragraph was not deleted
    if(deleted_line == false)
    {
        (*rowlist).Add(remv, ppos + offset - 1);
    }
    //else a paragraph was deleted
    else
    {
        (*doc).SetCursorY(prow - 1);
        int current_paragraph = (*visible).GetRow();
        (*visible).Add(current_paragraph);
        rowlist = (*visible).GetParagraph(current_paragraph);

        int currsize = (*rowlist).GetLine().size();

        //Deletes deleted paragraph from above paragraph
        for(int i = 0; i < lostline.size(); i++)
        {
            (*rowlist).Delete(currsize - lostline.size());
        }

        rowlist = (*visible).GetParagraph(current_paragraph + 1);

        //Recreates deleted paragraph
        for(int i = 0; i < lostline.size(); i++)
        {
            (*rowlist).Add(lostline[i], i);
        }
        (*doc).SetCursorY(prow);
    }
}

void NewLineCommand :: execute()
{
    //For undo/redo to assure right position/page
    if(set == false)
    {
        ppos = (*doc).GetCursorX();
        prow = (*doc).GetCursorY();
        ppage = (*visible).GetPage();
        set = true;
    }
    else
    {
        (*doc).SetCursorX(ppos);
        (*doc).SetCursorY(prow);
        (*visible).SetPage(ppage);
    }
    rowlist = (*visible).GetParagraph(current_paragraph);

    //currline = current paragraph
    std::string currline = (*rowlist).GetLine();

    //if in the middle of a paragraph
    if(ppos + offset < currline.size())
    {
        //store and delete characters in paragraph after cursor
        for(int i = ppos + offset; i < currline.size(); i++)
        {
            moved = moved + currline[i];
            (*rowlist).Delete(ppos + offset);
        }

        //Create new paragraph
        (*visible).Add(current_paragraph);

        //Fills new paragraph
        Rows* new_paragraph = (*visible).GetParagraph(current_paragraph + 1);
        for(int i = 0; i < moved.size(); i++)
        {
            (*new_paragraph).Add(moved[i], i);
        }
        (*doc).SetCursorX(0);

        if(prow + 1 > (*doc).GetRowNumInView() - 2)
        {
            (*visible).SetPage(ppage + 1);
            (*doc).SetCursorY(0);
        }
        else
        {
            (*doc).SetCursorY(prow + 1);
        }
    }
    //Else not in middle of paragraph
    else
    {
        (*visible).Add(current_paragraph);
        (*doc).SetCursorX(0);
        (*doc).SetCursorY(prow + 1);
        if(prow + 1 > (*doc).GetRowNumInView() - 2)
        {
            (*visible).SetPage(ppage + 1);
            (*doc).SetCursorY(0);
        }
    }
} 
void NewLineCommand :: unexecute()
{   
    Rows* old_paragraph = (*visible).GetParagraph(current_paragraph);
    for(int i = 0; i < moved.size(); i++)
    {
        (*old_paragraph).Add(moved[i], ppos + offset + i);
    }
    moved.clear();
    (*visible).Delete(current_paragraph + 1);
    (*doc).SetCursorX(ppos);
    (*doc).SetCursorY(prow);
    (*visible).SetPage(ppage);
}