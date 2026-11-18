# 로그인 화면 예시

[login.png](login.png)는 현재 소스를 Docker에서 실행해 얻은 로그인 출력을
PNG로 렌더링한 이미지입니다. 읽기 편하도록 글꼴과 색상을 적용했습니다.
실행 프로그램 자체가 이 색상이나 창 제목을 그리는 것은 아닙니다.

ASCII 로고는 `docs/examples/issue`의 문서용 예시입니다.
기본 `root/etc/issue`를 바꾸지 않고 아래처럼 읽기 전용 마운트로 재현할 수 있습니다.
저장소 최상위 디렉터리에서 실행하세요.

```sh
make docker-build
docker run --rm -it \
  -v "$PWD/docs/examples/issue:/bbs/root/etc/issue:ro" \
  drzzid:legacy
```

로그인 프롬프트가 나오면 `off`를 입력하여 종료할 수 있습니다.
이 화면 확인에는 계정을 만들 필요가 없습니다.
로그인 후 기능을 사용하려면 [설치 안내](../INSTALL.md)에 따라 계정을 생성하세요.

설치한 BBS의 배너를 바꾸려면 데이터 디렉터리의 `etc/issue`를 편집합니다.
접속 시 먼저 표시되는 안내 문구는 `etc/Welcome`에서 바꿀 수 있습니다.
