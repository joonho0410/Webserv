#!/usr/bin/env python
# -*- coding: utf-8 -*-
#import cgi
#import os

#upload_dir = "/path/to/upload/directory"

#form = cgi.FieldStorage()

#fileitem = form['fileToUpload']

#if fileitem.filename:
#    filepath = os.path.join(upload_dir, os.path.basename(fileitem.filename))

#    with open(filepath, 'wb') as f:
#        f.write(fileitem.file.read())

#    print("Content-Type: text/html")
#    print()
#    print("<html>")
#    print("<head>")
#    print("<title>파일 업로드</title>")
#    print("</head>")
#    print("<body>")
#    print("<h1>파일이 업로드되었습니다.</h1>")
#    print("</body>")
#    print("</html>")
#else:
#    print("Content-Type: text/html")
#    print()
#    print("<html>")
#    print("<head>")
#    print("<title>파일 업로드</title>")
#    print("</head>")
#    print("<body>")
#    print("<h1>파일 업로드에 실패하였습니다.</h1>")
#    print("</body>")
#    print("</html>")
print("HTTP/1.1 200 OK")
print("accept-ranges: bytes")
print("cache-control: public, max-age=0")
print("content-length: 200")
print("content-type: text/html; charset=UTF-8")
print("date: Wed, 04 Jul 2018 01:42:11 GMT")
print("last-modified: Wed, 04 Jul 2018 00:40:17 GMT")
print("x-powered-by: Express\r\n")

print("<h1>SAMPLE PYTHON SCRIPT!</h1>")