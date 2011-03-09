import sys
from d2c.Application import Application
from AMIToolsStub import AMIToolsFactoryStub

def main(argv=None):
    
    app = Application(AMIToolsFactoryStub())
    app.MainLoop()

if __name__ == "__main__":
    sys.exit(main())
