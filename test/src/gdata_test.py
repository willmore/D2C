try: 
  from xml.etree import ElementTree
except ImportError:  
  from elementtree import ElementTree
import gdata.spreadsheet.service
import gdata.service
import atom.service
import gdata.spreadsheet
import atom

from boto.ec2.cloudwatch import CloudWatchConnection
from d2c.data.CredStore import CredStore
from TestConfig import TestConfig
import boto
import sys
import boto.ec2.cloudwatch.metric
from boto.ec2.instance import Instance
import datetime
from mockito import *


gd_client = gdata.spreadsheet.service.SpreadsheetsService()
gd_client.email = 'chris.willmore@yahoo.com'
gd_client.password = '#######'
gd_client.source = 'exampleCo-exampleApp-1'
gd_client.ProgrammaticLogin()

feed = gd_client.GetSpreadsheetsFeed()
for i, entry in enumerate(feed.entry):
    if isinstance(feed, gdata.spreadsheet.SpreadsheetsCellsFeed):
      print '%s %s\n' % (entry.title.text, entry.content.text)
    elif isinstance(feed, gdata.spreadsheet.SpreadsheetsListFeed):
      print '%s %s %s' % (i, entry.title.text, entry.content.text)
      # Print this row's value for each column (the custom dictionary is
      # built from the gsx: elements in the entry.) See the description of
      # gsx elements in the protocol guide.
      print 'Contents:'
      for key in entry.custom:
        print '  %s: %s' % (key, entry.custom[key].text)
      print '\n',
    else:
      print '%s %s\n' % (i, entry.title.text)
