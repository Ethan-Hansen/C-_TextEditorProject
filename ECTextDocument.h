#ifndef ECTextDocument_h
#define ECTextDocument_h

#include "ECTextViewImp.h"
#include <fstream>

/* *******************************************
    Component:
    Interface for composite design pattern 
    implementation in Composite, Rows, and ECVisible
******************************************* */
class Component
{
public:
    virtual ~Component() {}
    virtual void Delete(int row) = 0;
};
/* *******************************************
    Rows Object:
    This object stores the current contents of
    the text editor in a vector of strings.
    Is updated after every new editing command.
******************************************** */
class Rows : public Component
{
public:
    Rows();
    ~Rows();
    void Add(char x, int pos);
    void Delete(int pos);
    int GetSize();
    int GetLastSpace(int pos);
    int GetNextSpace(int pos);
    std::string GetLine();

private:
    std::string line;
};

/* ********************************************
    ECVisible:
    This is the composite object for the composite
    design pattern.  Stores paragraphs as Rows objects
    int paragraphs private member.
    Stores what is displayed in display private member,
    which is updated with each call to update
******************************************** */
class ECVisible : public Component
{
public:
    ECVisible(ECTextViewImp* view);
    ~ECVisible();

    void Add(int row);
    void Delete(int row);

    //Returns paragraph at index
    Rows* GetParagraph(int index);
    //Returns offset of row in view relative to paragraph in paragraphs
    int GetOffset();

    //Returns the location of current paragraph in paragraphs private member
    int GetRow();

    int PreviousRowSize();
    int NextRowSize();

    //Returns size of paragraphs member
    int GetSize();

    //Returns size of display private member
    int GetDisplaySize();

    int GetPage();
    int GetDisplayLineSize(int row);
    void Update();
    void SetPage(int x);
    void ReadFile(char* filename);
    void WriteFile();
private:
    std::vector<Rows*> paragraphs;
    std::vector<std::vector<std::string>> display;
    ECTextViewImp* doc;
    char* filename;

    int page = 0;
};

/* ******************************************** 
    Command Design Pattern Implementation:
    Each command corresponds to an editing action
    that needs to be able to be redone and undone
********************************************* */
class ECCommand
{
public:
    virtual ~ECCommand() {}
    virtual void execute() = 0;
    virtual void unexecute() = 0;
};

class InsertCommand : public ECCommand
{
public:
    InsertCommand(ECVisible* paragraphs, ECTextViewImp* view, Rows* rows, char x, int off, int para) : visible(paragraphs), doc(view), rowlist(rows), ins(x), offset(off), current_paragraph(para) {} 
    ~InsertCommand() {}
    void execute();
    void unexecute();
private:
    int offset;
    char ins;
    int ppos;
    int prow;
    int frow;
    int ppage;
    int current_paragraph;
    std::string lost;
    ECTextViewImp* doc;
    Rows* rowlist;
    ECVisible* visible;
    bool set = false;
};

class RemoveCommand : public ECCommand
{
public:
    RemoveCommand(ECVisible* paragraphs, ECTextViewImp* view, Rows* rows, int off, int para) : visible(paragraphs), doc(view), rowlist(rows), offset(off), current_paragraph(para) {}
    ~RemoveCommand() {}
    void execute();
    void unexecute();
private:
    char remv;
    bool set = false;
    std::string lostline = "";
    bool deleted_line = false;
    int prow;
    int ppos;
    int offset;
    int ppage;
    int current_paragraph;
    ECTextViewImp* doc;
    Rows* rowlist;
    ECVisible* visible;
};

class NewLineCommand : public ECCommand
{
public:
    NewLineCommand(ECTextViewImp* view, Rows* rows, ECVisible* paragraphs, int off, int para) : doc(view), rowlist(rows), visible(paragraphs), offset(off), current_paragraph(para) {}
    ~NewLineCommand() {}
    void execute();
    void unexecute();
private:
    int prow;
    int ppos;
    int offset;
    int ppage;
    int current_paragraph;
    bool set = false;
    std::string moved = "";
    ECTextViewImp* doc;
    Rows* rowlist;
    ECVisible* visible;
};
/* *******************************************
    ECCommandHistory Object:
    Containts two vectors of commands.  All created
    commands are pushed onto the done vector.
    For undo, they are pushed off done and put on undone.
    For redo, they are pushed off undone and put on done.
    When a new command is added to done, undone is cleared.
******************************************* */
class ECCommandHistory
{
public:
    ECCommandHistory();
    ~ECCommandHistory();
    void Undo();
    void Redo();
    void ExecuteCommand(ECCommand* com);
private:
    std::vector<ECCommand*> done;
    std::vector<ECCommand*> undone;
};

/*********************************************
    KeyHandler Object:
    The base handler passes the keypress along
    to each subsequent handler in the chain until
    it is handled.  If it is not handled, nothing
    happens.
******************************************* */
class KeyHandler
{
public:
    virtual ~KeyHandler()
    {
        if(nxt_Handler)
        {
            delete nxt_Handler;
        }
    }
    void SetHistory(ECCommandHistory* hist)
    {
        comHist = hist;
    }
    void SetVisible(ECVisible * temp)
    {
        visible = temp;
    }
    void SetView(ECTextViewImp * view)
    {
        doc = view;
    }
    void SetNext(KeyHandler*  nxt)
    {
        nxt_Handler = nxt;
        (*nxt_Handler).SetHistory(comHist);
        (*nxt_Handler).SetView(doc);
        (*nxt_Handler).SetVisible(visible);
    }
    virtual void Handle(int key)
    {
        if(nxt_Handler != nullptr)
        {
            (*nxt_Handler).Handle(key);
        }
        else
        {
        }
    }
protected:
    ECTextViewImp * doc = nullptr;
    ECVisible * visible = nullptr;
    ECCommandHistory * comHist = nullptr;
private:
    KeyHandler * nxt_Handler = nullptr;
};

class QuitHandler : public KeyHandler
{
public:

    void Handle(int key);
};

class NewLineHandler : public KeyHandler
{
    void Handle(int key);
};

class UndoHandler : public KeyHandler
{
    void Handle(int key);
};

class RedoHandler : public KeyHandler
{
    void Handle(int key);
};

class RemoveHandler : public KeyHandler
{
    void Handle(int key);
};

class CursorHandler : public KeyHandler
{
    void Handle(int key);
};

class InsertHandler : public KeyHandler
{
    void Handle(int key);
};


/*********************************************
    ECTextDocumentCtrl Object:
    This object holds all of the necessary pointers and
    acts as the initial object.  Inherits from observer so
    is notified with every keypress.
*********************************************/
class ECTextDocumentCtrl : public ECObserver
{
public:
    ECTextDocumentCtrl(char* filename);
    ~ECTextDocumentCtrl();
    void Update();
private:
    ECCommandHistory* comHist;

    ECTextViewImp* doc;

    ECVisible* visible;

    KeyHandler* Handler;
};

#endif