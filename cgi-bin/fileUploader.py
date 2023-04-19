#!/usr/bin/env python
# -*- coding: utf-8 -*-
import cgi
import os
import sys

print("HTTP/1.1 200 OK")
print("accept-ranges: bytes")
print("cache-control: public, max-age=0")
print("content-type: text/html; charset=UTF-8")
print("x-powered-by: Express\r\n")


# 파일이 저장될 디렉토리가 존재하지 않으면 생성합니다.
file_storage_dir = "./FileStorage"
if not os.path.exists(file_storage_dir):
    os.makedirs(file_storage_dir)

# 환경변수에서 요청 메서드와 request body를 가져옴
request_method = os.environ['REQUEST_METHOD']
request_body_size = int(os.environ.get('CONTENT_LENGTH', 0))

# 요청 메서드가 POST이고 request body가 존재하는 경우에만 실행
if request_method == 'POST' :
    # request body에서 파일 데이터 추출
    form = cgi.FieldStorage(environ=os.environ)
    # 파일이 여러개일 수 있으므로 각각의 파일을 반복하며 저장
    for key in form:
        fileitem = form[key]
        if fileitem.filename:
            # 파일 이름과 경로 지정
            filename = os.path.basename(fileitem.filename)
            filepath = file_storage_dir + "/" + filename
            # 파일 저장
            with open(filepath, 'wb') as f:
                f.write(fileitem.file.read())
            print("File '%s' saved successfully.\n" % filename)

print("<h1>SAMPLE PYTHON SCRIPT!</h1>")