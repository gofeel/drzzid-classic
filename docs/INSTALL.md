# Ubuntu / Debian

```sh
sudo apt-get update
sudo apt-get install --no-install-recommends build-essential libncurses-dev libcrypt-dev
make
make test
```

코어는 Linux 전용입니다. macOS에서는 선택적인 Docker 빌드를 사용하세요.
전용 일반 계정의 작업 디렉터리에 root/ 템플릿을 복사하고 실행 파일을 배치합니다.
아래 경로는 테스트용 예이며 기존 데이터 디렉터리에 덮어쓰지 마세요.

```sh
umask 077
mkdir -p "$HOME/drzzid-data"
cp -R root/. "$HOME/drzzid-data/"
mkdir -p "$HOME/drzzid-data"/{bin,boards,mail,tmp,msgtmp,chat,vote,signatures,plans}
cp src/bbs src/bbs.chatd src/bbsrf src/SaveToCabinet src/cntuser "$HOME/drzzid-data/bin/"
export BBSHOME="$HOME/drzzid-data"
cd "$BBSHOME"
```

원본 adduser 사용법은 저장소의 src/adduser를 인자 없이 실행해 확인합니다.
새 .PASSWDS에 SYSOP 및 필요한 계정을 만든 뒤 `./bin/bbs`를 실행합니다.
원본 adduser는 암호를 명령 인자로 받습니다. DES는 첫 8바이트만 유효합니다.
외부 명령 실행 등 원본 기능을 포함하므로 격리된 복원·개발 환경에서 사용하세요.
원본 설치 스크립트 대신 위 절차를 사용합니다. BBS 셸을 시스템 로그인 셸로 등록하지 마세요.

## 선택: Docker

```sh
make docker-build
docker run --rm -it --entrypoint sh drzzid:legacy
```

컨테이너의 /bbs/root에서 /bbs/src/adduser로 테스트 계정을 만든 뒤 /bbs/src/bbs를 실행할 수 있습니다.
계정·데이터를 이미지에 넣지 않으며, 호스트 포트를 공개하지 않습니다.
기존 강화 버전의 .auth 해시는 이 복원판에서 사용할 수 없습니다.
