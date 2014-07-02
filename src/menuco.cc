/* The menu commands in FOX are designed for static menus. i.e. you
   can't dynamically generate menu entries. Actually you can, but it's
   cumbersome.  A better way is to subclass the menu command class and
   let the menu command do the necessary actions by itself.

   These actions may also cause the menu command to be deleted. This seems
   to be fine. The handle() functions in the widgets don't refer to the
   object, they simply return. This could change in future releases of
   FOX but for now it's safe. */

namespace FX
{

class FXAPI FYMenuCommand : public FXMenuCommand
{
  FXDECLARE(FYMenuCommand)
protected:
  FYMenuCommand();
private:
  FYMenuCommand(const FYMenuCommand&);
  FYMenuCommand &operator=(const FYMenuCommand&);

  void (*a_func)(int);
  int a_arg;
public:
  FYMenuCommand(FXComposite* p,
                const FXString& text,FXIcon* ic=NULL,
                FXuint opts=0, 
                void (*afun)(int)= NULL, int aarg= 0);
  long run_func(FXObject* sender,FXSelector sel,void* ptr);

  virtual ~FYMenuCommand();
};
}

using namespace FX;

FXDEFMAP(FYMenuCommand) FYMenuCommandMap[]=
{
  FXMAPFUNC(SEL_COMMAND, 2216, FYMenuCommand::run_func),
};


FXIMPLEMENT(FYMenuCommand,FXMenuCommand,FYMenuCommandMap,
            ARRAYNUMBER(FYMenuCommandMap))

FYMenuCommand::FYMenuCommand(){ }
FYMenuCommand::~FYMenuCommand() { }

FYMenuCommand::FYMenuCommand
             (FXComposite* p,
              const FXString& text,FXIcon* ic,
              FXuint opts,
              void (*afun)(int), int aarg)
   : FXMenuCommand(p, text, ic, this, 2216, opts)
{
  a_func= afun;
  a_arg= aarg;
}

long FYMenuCommand::run_func(FXObject* sender,FXSelector sel,void* ptr)
{
  if (a_func) (*a_func)(a_arg);
  return 1;
}

