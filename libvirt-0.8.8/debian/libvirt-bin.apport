'''apport package hook for libvirt-bin

(c) 2009 Canonical Ltd.
Author:
Jamie Strandboge <jamie@ubuntu.com>

'''

from apport.hookutils import *
from os import path
import re

def recent_kernlog(pattern):
    '''Extract recent messages from kern.log or message which match a regex.
       pattern should be a "re" object.  '''
    lines = ''
    if os.path.exists('/var/log/kern.log'):
        file = '/var/log/kern.log'
    elif os.path.exists('/var/log/messages'):
        file = '/var/log/messages'
    else:
        return lines

    for line in open(file):
        if pattern.search(line):
            lines += line
    return lines

def recent_auditlog(pattern):
    '''Extract recent messages from kern.log or message which match a regex.
       pattern should be a "re" object.  '''
    lines = ''
    if os.path.exists('/var/log/audit/audit.log'):
        file = '/var/log/audit/audit.log'
    else:
        return lines

    for line in open(file):
        if pattern.search(line):
            lines += line
    return lines

def add_info(report):
    attach_conffiles(report, 'libvirt-bin')
    attach_related_packages(report, ['apparmor', 'libapparmor1',
        'libapparmor-perl', 'apparmor-utils', 'auditd', 'libaudit0'])

    # get apparmor stuff. copied from source_apparmor.py until apport runs
    # runs hooks via attach_related_packages
    attach_file(report, '/proc/version_signature', 'ProcVersionSignature')
    attach_file(report, '/proc/cmdline', 'ProcCmdline')

    sec_re = re.compile('audit\(|apparmor|selinux|security', re.IGNORECASE)
    report['KernLog'] = recent_kernlog(sec_re)

    if os.path.exists("/var/log/audit"):
        # this needs to be run as root
        report['AuditLog'] = recent_auditlog(sec_re)

