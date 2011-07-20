import wx

class FicheFrameWithFocus( wx.Frame ) :

    def __init__( self ) :

        wx.Frame.__init__( self, None, -1 ,"FicheFrameWithFocus", size=(275, 500) )

        self.scroll = wx.ScrolledWindow( self, -1 )

        self.frmPanel = wx.Panel( self.scroll, -1 )

        # A sizer allows the horizontal scrollbar to appear when needed.
        flGrSzr = wx.FlexGridSizer( 50, 2, 2, 2 )   # rows, cols, hgap, vgap

        for i in range( 50 ) :

            textStr = " Champ (txt) %2d  : " % (i+1)
            flGrSzr.Add( wx.StaticText( self.frmPanel, -1, textStr ) )

            txtCtrl = wx.TextCtrl( self.frmPanel, -1, size=(100, -1), style=0 )
            wx.EVT_SET_FOCUS( txtCtrl, self.OnFocus )
            flGrSzr.Add( txtCtrl )

        #end for

        # Create a border around frmPnlSizer to the edges of the frame.
        frmPnlSizer = wx.BoxSizer( wx.VERTICAL )
        frmPnlSizer.Add( flGrSzr, proportion=1, flag=wx.ALL, border=20 )

        self.frmPanel.SetSizer( frmPnlSizer )
        self.frmPanel.SetAutoLayout( True )
        self.frmPanel.Layout()
        self.frmPanel.Fit()     # frmPnlSizer borders will be respected

        self.Center()
        self.MakeModal( True )

        # Scrolling parameters must be set AFTER all controls have been laid out.
        self.frmPanelWid, self.frmPanelHgt = self.frmPanel.GetSize()
        self.unit = 1
        self.scroll.SetScrollbars( self.unit, self.unit, self.frmPanelWid/self.unit, self.frmPanelHgt/self.unit )

        

    #end __init__ def

    def OnFocus( self, event ) :
        """
        This makes the selected (the one in focus) textCtrl to be automatically
        repositioned to the top-left of the window. One of the scrollbars
        must have been moved for this to happen.

        The benefits for doing this are dubious. but this demonstrates how it can be done.
        """

        parentControl = self.FindWindowById( event.GetId() )  # The parent container control
        parentPosX, parentPosY = parentControl.GetPosition()
        hx, hy = parentControl.GetSizeTuple()
        clientSizeX, clientSizeY = self.scroll.GetClientSize()

        sx, sy = self.scroll.GetViewStart()
        magicNumber = 20        # Where did this value come from ?!
        sx = sx * magicNumber
        sy = sy * magicNumber

        if (parentPosY < sy ) :
            self.scroll.Scroll( 0, parentPosY/self.unit )

        if ( parentPosX < sx ) :
             self.scroll.Scroll( 0, -1 )

        if (parentPosX + sx) > clientSizeX  :
            self.scroll.Scroll( 0, -1 )

        if (parentPosY + hy - sy) > clientSizeY :
            self.scroll.Scroll( 0, parentPosY/self.unit )

    #end OnFocus def

#end FicheFrameWithFocus class

if __name__ == '__main__' :

    myapp = wx.App( redirect=False )
    appFrame = FicheFrameWithFocus()
    appFrame.Show()
    myapp.MainLoop()