import os

# 파일 경로 및 파일명 설정
file_path = "/path/to/file"
file_name = "example.txt"

# 파일 권한 확인
if os.access(f"{file_path}/{file_name}", os.W_OK):
    # 권한이 있는 경우 파일 삭제
    os.remove(f"{file_path}/{file_name}")
    print(f"{file_path}/{file_name} has been deleted.")
else:
    # 권한이 없는 경우 에러 메시지 출력
    print(f"You don't have permission to delete {file_path}/{file_name}.")