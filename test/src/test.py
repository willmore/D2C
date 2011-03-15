import wx
import wx.wizard as wizmod
import os.path
padding = 5

class wizard_page(wizmod.PyWizardPage):
    ''' An extended panel obj with a few methods to keep track of its siblings.  
        This should be modified and added to the wizard.  Season to taste.'''
    def __init__(self, parent, title):
        wx.wizard.PyWizardPage.__init__(self, parent)
        self.next = self.prev = None
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        title = wx.StaticText(self, -1, title)
        title.SetFont(wx.Font(18, wx.SWISS, wx.NORMAL, wx.BOLD))
        self.sizer.AddWindow(title, 0, wx.ALIGN_LEFT|wx.ALL, padding)
        self.sizer.AddWindow(wx.StaticLine(self, -1), 0, wx.EXPAND|wx.ALL, padding)
        self.SetSizer(self.sizer)

    def add_stuff(self, stuff):
        'Add aditional widgets to the bottom of the page'
        self.sizer.Add(stuff, 0, wx.EXPAND|wx.ALL, padding)

    def SetNext(self, next):
        'Set the next page'
        self.next = next

    def SetPrev(self, prev):
        'Set the previous page'
        self.prev = prev

    def GetNext(self):
        'Return the next page'
        return self.next

    def GetPrev(self):
        'Return the previous page'
        return self.prev


class wizard(wx.wizard.Wizard):
    'Add pages to this wizard object to make it useful.'
    def __init__(self, title, img_filename=""):
        # img could be replaced by a py string of bytes
        if img_filename and os.path.exists(img_filename):
                img = wx.Bitmap(img_filename)
        else:   img = wx.NullBitmap
        wx.wizard.Wizard.__init__(self, None, -1, title, img)
        self.pages = []
        # Lets catch the events
        self.Bind(wizmod.EVT_WIZARD_PAGE_CHANGED, self.on_page_changed)
        self.Bind(wizmod.EVT_WIZARD_PAGE_CHANGING, self.on_page_changing)
        self.Bind(wizmod.EVT_WIZARD_CANCEL, self.on_cancel)
        self.Bind(wizmod.EVT_WIZARD_FINISHED, self.on_finished)

    def add_page(self, page):
        'Add a wizard page to the list.'
        if self.pages:
            previous_page = self.pages[-1]
            page.SetPrev(previous_page)
            previous_page.SetNext(page)
        self.pages.append(page)

    def run(self):
        self.RunWizard(self.pages[0])

    def on_page_changed(self, evt):
        'Executed after the page has changed.'
        if evt.GetDirection():  dir = "forward"
        else:                   dir = "backward"
        page = evt.GetPage()
        print "page_changed: %s, %s\n" % (dir, page.__class__)

    def on_page_changing(self, evt):
        'Executed before the page changes, so we might veto it.'
        if evt.GetDirection():  dir = "forward"
        else:                   dir = "backward"
        page = evt.GetPage()
        print "page_changing: %s, %s\n" % (dir, page.__class__)

    def on_cancel(self, evt):
        'Cancel button has been pressed.  Clean up and exit without continuing.'
        page = evt.GetPage()
        print "on_cancel: %s\n" % page.__class__

        # Prevent cancelling of the wizard.
        if page is self.pages[0]:
            wx.MessageBox("Cancelling on the first page has been prevented.", "Sorry")
            evt.Veto()

    def on_finished(self, evt):
        'Finish button has been pressed.  Clean up and exit.'
        print "OnWizFinished\n"


if __name__ == '__main__':

    app = wx.PySimpleApp()  # Start the application

    # Create wizard and add any kind pages you'd like
    mywiz = wizard('Simple Wizard', img_filename='wiz.png')
    page1 = wizard_page(mywiz, 'Page 1')  # Create a first page
    page1.add_stuff(wx.StaticText(page1, -1, 'Hola'))
    mywiz.add_page(page1)

    # Add some more pages
    mywiz.add_page( wizard_page(mywiz, 'Page 2') )
    mywiz.add_page( wizard_page(mywiz, 'Page 3') )

    mywiz.run() # Show the main window

    # Cleanup
    mywiz.Destroy()
    app.MainLoop()
