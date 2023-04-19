#!/usr/bin/env python
# -*- coding: utf-8 -*-
import cgi
import os
import sys

# 파일 경로 및 파일명 설정
file_path = "./FileStorage"
file_name = "test.txt"


# 파일 권한 확인 파일이 애초에 존재하지 않을 때는 404 not found
if os.access("{}/{}".format(file_path, file_name), os.W_OK):
    # 권한이 있는 경우 파일 삭제
    os.remove("{}/{}".format(file_path, file_name))
    print("{}/{} has been deleted.".format(file_path, file_name))
else:
    # 권한이 없는 경우 에러 메시지 출력 403 forbidden
    print("You don't have permission to delete {}/{}.".format(file_path, file_name))

print("<h1>FILE DELETE IS COMPLETE !</h1>")