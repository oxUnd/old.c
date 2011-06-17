#!/usr/bin/env python
import os
import re
import urllib2
def uniq(list):
    set = {}
    return [ set.setdefault(x,x) for x in list if x not in set ]
def get_groups_id(url):
    req = urllib2.urlopen(url)
    s = req.read()
    req.close()
    if s == '':
        return None
    #names = re.findall('<a href="%s/.*?\.shtml" target="_blank" title="(.*?)">' % (url), s)
    ids = re.findall('manhua/(.*?)\.shtml', s)
    #for i in range(0, len(names)):
        #print "%s:%s " % (names[i], ids[i])
    #exit()
    return ids
def get_page(url, g_id = None):
    req = urllib2.urlopen(url + '/' + g_id + '.shtml')
    s = req.read()
    req.close()
    return re.findall('(%s_\d+\.shtml)' % (g_id), s);
def get_image(url):
    req = urllib2.urlopen(url)
    s = req.read()
    req.close()
    return re.findall('<a href=\'.*?\.shtml\'><img.*?src="(.*?)".*?/></a>', s)

if __name__ == '__main__':
    ids = get_groups_id('http://www.yaojingweiba.com/manhua')
    ids = uniq(ids)
    num = len(ids)
    i = 0
    for g_id in ids:
        print "%d / %d" % (i, num)
        i = i + 1
        fp = open('%s.txt' % (g_id), 'a');
        print g_id
        pages =  get_page('http://www.yaojingweiba.com/manhua', g_id)
        pages.append('%s.shtml' % (g_id)) # first page
        pages = uniq(pages)
        for p_id in pages:
            print p_id
            images = get_image('http://www.yaojingweiba.com/manhua/%s' % (p_id))
            if images:
                fp.writelines(images[0] + "\n")
        fp.close()
    os.system('sh ./cn.sh')
    os.system('sh ./get.sh')
    print 'Done'
