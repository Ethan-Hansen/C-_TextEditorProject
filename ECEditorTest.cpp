// Test code for editor
#include "ECTextViewImp.h"
#include "ECTextDocument.h"
#include "ECObserver.h"
#include <iostream>

using namespace  std;

int myCounter = 0;

int main(int argc, char *argv[])
{
    ECTextDocumentCtrl* q = new ECTextDocumentCtrl(argv[1]);
    delete q;
}
