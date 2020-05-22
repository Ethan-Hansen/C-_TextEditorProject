#include "ECTextViewImp.h"
#include "ECTextDocument.h"
#include "ECObserver.h"
#include <vector>
#include <iostream>
#include <string>
/* ***************************
    ECCOMMANDHISTORY METHODS
*************************** */
ECCommandHistory :: ECCommandHistory() {}
ECCommandHistory :: ~ECCommandHistory()
{
    for(int i = 0; i < done.size(); i++)
    {
        delete done[i];
    }
    for(int i = 0; i < undone.size(); i++)
    {
        delete undone[i];
    }
}
void ECCommandHistory :: Undo()
{
    if(done.size() > 0)
    {
        ECCommand* temp = done[done.size() - 1];
        undone.push_back(temp);
        (*temp).unexecute();
        done.pop_back();
    }
}
void ECCommandHistory :: Redo()
{
    if(undone.size() > 0)
    {
        ECCommand * temp = undone[undone.size() - 1];
        done.push_back(temp);
        (*temp).execute();
        undone.pop_back();
    }
}
void ECCommandHistory :: ExecuteCommand(ECCommand* com)
{
    //Clears past things that were undone
    for(int i = 0; i < undone.size(); i++)
    {
        delete undone[i];
    }
    undone.clear();
    (*com).execute();
    done.push_back(com);
}

/* ********************************
    ECTEXTDOCUMENTCTRL METHODS
******************************** */
ECTextDocumentCtrl :: ECTextDocumentCtrl(char* filename)
{   
    //Initializes the view, rowlist and command history
    comHist = new ECCommandHistory();

    doc = new ECTextViewImp();
    (*doc).Init();
    (*doc).AddRow("");

    visible = new ECVisible(doc);

    //Reads file and adds to ECVisible paragraphs member
    (*visible).ReadFile(filename);
    (*visible).Update();

    (*doc).SetCursorX(0);
    (*doc).SetCursorY(0);

    //Constructs the Handler 
    Handler = new KeyHandler();
    (*Handler).SetHistory(comHist);
    (*Handler).SetVisible(visible);
    (*Handler).SetView(doc);
    QuitHandler* h1 = new QuitHandler();
    NewLineHandler* h2 = new NewLineHandler();
    UndoHandler* h3 = new UndoHandler();
    RedoHandler* h4 = new RedoHandler();
    RemoveHandler* h5 = new RemoveHandler();
    CursorHandler* h6 = new CursorHandler();
    InsertHandler* h7 = new InsertHandler();
    (*Handler).SetNext(h1);
    (*h1).SetNext(h2);
    (*h2).SetNext(h3);
    (*h3).SetNext(h4);  
    (*h4).SetNext(h5);
    (*h5).SetNext(h6);
    (*h6).SetNext(h7);

    (*doc).Attach(this);
    (*doc).Show();
}
ECTextDocumentCtrl :: ~ECTextDocumentCtrl()
{
    (*visible).WriteFile();
    (*doc).InitRows();
    delete doc;
    delete comHist;
    delete visible;
    delete Handler;
}
void ECTextDocumentCtrl :: Update()
{
    (*Handler).Handle((*doc).GetPressedKey());
}

/* ***************************
    ROWS METHODS
*************************** */
Rows :: Rows()
{
    line = "";
}
Rows :: ~Rows() {}
void Rows :: Add(char x, int pos)
{
    line.insert(pos, 1, x);
}
void Rows :: Delete(int pos)
{
    line.erase(pos, 1);
}
int  Rows :: GetSize()
{
    int x = line.size();
    return x; 
}
//Gets last space before pos
int Rows :: GetLastSpace(int pos)
{
    int last_space = 0;
    for(int i = 0; i < pos; i++)
    {
        if(line[i] == ' ')
        {
            last_space = i;
        }
    }
    return last_space;
}
//Gets first space after pos
int Rows :: GetNextSpace(int pos)
{
    int nxt_space = pos;
    for(int i = pos; i < line.size(); i++)
    {
        if(line[i] == ' ')
        {
            return i;
        }
        else
        {
            nxt_space = i;
        }
    }
    return nxt_space;
}
std::string Rows :: GetLine()
{
    return line;
}

/* *************************************
    ECVISIBLE IMPLEMENTATION
************************************* */
ECVisible :: ECVisible(ECTextViewImp* view)
{
    paragraphs.push_back(new Rows());
    doc = view;
}

ECVisible :: ~ECVisible()
{
    for(int i = 0; i < paragraphs.size(); i++)
    {
        delete paragraphs[i];
    }
}
//Adds new paragraph at row
void ECVisible :: Add(int row)
{
    auto iter = paragraphs.begin();
    for(int i = 0; i < row + 1; i++)
    {
        iter++;
    }
    paragraphs.insert(iter, new Rows());
}
//Deletes paragraph at row
void ECVisible :: Delete(int row)
{
    auto iter = paragraphs.begin();
    for(int i = 0; i < row; i++)
    {
        iter++;
    }
    delete paragraphs[row];
    paragraphs.erase(iter);
}
//Returns Paragraph
Rows* ECVisible :: GetParagraph(int index)
{
    return paragraphs[index];
}
//Gets the offset of the current paragraph relative to the position in the view
int ECVisible :: GetOffset()
{
    int total = 0;
    int row = (*doc).GetCursorY() + (((*doc).GetRowNumInView() - 1) * page);
    for(int i = 0; i < display.size(); i++)
    {
        if(row < display[i].size())
        {
            for(int k = 0; k < row; k++)
            {
                total = total + display[i][k].size();
            }
            return total;
        }
        else
        {
            row = row - display[i].size();
        }
    }

    return total;
}
//Gets the current paragraph based off position in view
int ECVisible :: GetRow()
{
    int row = (*doc).GetCursorY() + (((*doc).GetRowNumInView() - 1) * page);

    for(int i = 0; i < display.size(); i++)
    {
        if(row < display[i].size())
        {
            return i;
        }
        else
        {
            row = row - display[i].size();
        }
    }

    return 0;
}
//Gets size of line above current line in view
int ECVisible :: PreviousRowSize()
{
    int row = (*doc).GetCursorY() - 1 + (((*doc).GetRowNumInView() - 1) * page);
    for(int i = 0; i < display.size(); i++)
    {
        if(row < display[i].size())
        {
            return display[i][row].size();
        }
        else
        {
            row = row - display[i].size();
        }
    }

    return 0;
}
//Gets size of next line below current line in view
int ECVisible :: NextRowSize()
{
    int row = (*doc).GetCursorY() + 1 + (((*doc).GetRowNumInView() - 1) * page);
    for(int i = 0; i < display.size(); i++)
    {
        if(row < display[i].size())
        {
            return display[i][row].size();
        }
        else
        {
            row = row - display[i].size();
        }
    }

    return 0;
}
//Gets  the number of paragraphs
int ECVisible :: GetSize()
{
    return paragraphs.size();
}
//Gets the number of lines in view
int ECVisible :: GetDisplaySize()
{
    int size = 0;
    for(int i = 0; i < display.size(); i++)
    {
        size = size + display[i].size();
    }
    return size;
}
//Updates view and display member
void ECVisible :: Update()
{
    display.clear();
    for(int i = 0; i < paragraphs.size(); i++)
    {
        std::vector<std::string> newrow;
        std::string currline = (*paragraphs[i]).GetLine();
        std::string newline = "";
        std::string newword = "";
        for(int k = 0; k < currline.size(); k++)
        {
            newword = newword + currline[k];
            if(currline[k] == ' ')
            {
                if(newline.size() + newword.size() > (*doc).GetColNumInView())
                {
                    newrow.push_back(newline);
                    newline = newword;
                    newword = "";
                }
                else
                {
                    newline = newline + newword;
                    newword = "";
                }
            }
        }
        if(newword != "")
        {
            if(newline.size() + newword.size() > (*doc).GetColNumInView())
            {
                newrow.push_back(newline);
                newrow.push_back(newword);
            }
            else
            {
                newline = newline + newword;
                newrow.push_back(newline);
            }
            
        }
        else
        {
            newrow.push_back(newline);
        }
        display.push_back(newrow);
    }

    (*doc).InitRows();
    (*doc).AddRow("");
    int rowcounter = 0;
    for(int i = 0; i < display.size(); i++)
    {
        for(int k = 0; k < display[i].size(); k++)
        {
            if(rowcounter < (((*doc).GetRowNumInView() - 1) * page))
            {
                rowcounter++;
            }
            else
            {
                if(rowcounter < (((*doc).GetRowNumInView() - 1) * page) + (*doc).GetRowNumInView() - 1)
                {
                    (*doc).AddRow(display[i][k]);
                    rowcounter++;
                }
            }
        }
    }
}

void ECVisible :: SetPage(int x)
{
    page = x;
}

int ECVisible :: GetPage()
{
    return page;
}

//Gets size of line where cursor is in display
int ECVisible :: GetDisplayLineSize(int row)
{
    row = row + (((*doc).GetRowNumInView() - 1) * page);
    int para = 0;
    int line = 0;
    for(int i = 0; i < display.size(); i++)
    {
        if(row < display[i].size())
        {
            para = i;
            line = row;
            return display[para][line].size();
        }
        else
        {
            row = row - display[i].size();
        }
    }
    return display[para][line].size();
}

//Reads file denoted by name and stores in paragraphs member
void ECVisible :: ReadFile(char* name)
{
    filename = name;
    std::ifstream file;
    file.open(filename, std::ifstream::in);
    if(file.is_open())
    {
        int count = 0;
        for(std::string line; std::getline(file, line);)
        {
            Add(count);
            Rows* rowlist = GetParagraph(count);
            for (int i = 0; i < line.size(); i++)
            {
                (*rowlist).Add(line[i], i);
            }
            count++; 
        }
        Delete(count);
        file.close();
    }
}

//Writes paragraphs to filename
void ECVisible :: WriteFile()
{
    std::ofstream file;
    file.open(filename, std::ofstream::out | std::ofstream::trunc);
    for(int i =0; i < paragraphs.size(); i++)
    {
        Rows* line = paragraphs[i];
        std::string add = (*line).GetLine();
        file << add;
        if(i != paragraphs.size() - 1)
        {
            file << '\n';
        }
    }
    file.close();
}