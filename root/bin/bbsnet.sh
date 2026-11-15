#!/bin/sh 
#
#	define the standout() and standend() escape sequences...
#	사용: bbsnet.sh <utmp num>
SO="[7m"
SE="[0m"

trap '\rm -f pingerror.tmp ; exit' 0 1 2 3 15

stty pass8 -istrip cs8
stty erase 

RLOGIN=/usr/bin/rlogin
TELNET=/usr/bin/telnet
MORE=/usr/bin/more
HEAD=/usr/bin/head
RSH=/usr/bin/rsh
PING=/bin/ping
CLEAR=/usr/bin/clear
FINGER=/usr/bin/finger
pingtimeout=10

while true
do

$CLEAR
if [ `bin/cntuser .UTMP` -gt $MAX_SUIT_ACTIVE ]; then
echo "[말림비]/[나들이]

     없습니다. 다음 주소로 ${SO}직접 접속${SE}하십시오.


"
echo -n "Return을 누르세요.. "
read CHOICE
exit
fi

echo "[DRYBBS]/[나들이]

${SO}번호   BBS 이름                     번호  BBS 이름                              ${SE} 
  [1]   posb                          
  [2]   미리내                       
  [3]   vnet                       
  [4]   ctrl-d                   
# [5]   정웅비             
  [6]   고답(미주알고주알)
  [7]   Green
  [8]   Shell
  [9]   J-bbs
 [10]   Lion

 [89]   ${SO}직접입력${SE}
 [99]   말림비 부속 수족관
 [ q]   메인메뉴로...... 
"
echo -n "골라골라.....[q]: " \

# 세팅해야 할 변수들
# protocol		S=script, F=finger, T=telnet, R=rlogin R=mud
# targethostname	symbolic host name
# targethostip		dotted decimal
# username
# scriptfile		maybe 'bin/XXXXXX'
# service		bbslog파일에 기록될 내용 한 줄
# targetport		머드할 때 사용되는 포트

if read CHOICE
then
    protocol=""
    targethostname=""
    targethostip=""
    username=""
    scriptfile=""
    service="나들이"
    targetport=""
    case $CHOICE in
    	'')
	    ${CLEAR}
	    exit
	    ;;

	q) 
	    ${CLEAR}
	    exit
	    ;;
    esac
    $CLEAR
    echo "[DRYBBS]/[나들이]"
    case "${CHOICE}" in
	'')
	    protocol="N"
	    ;;
	1)
	    service="N:포스비"
	    targethostname="posb.postech.ac.kr"
	    protocol="R"
	    username="posb"
	    ;;
	
	2)
	    service="N:미리내"
	    targethostname="mirinae.postech.ac.kr"
	    protocol="R"
	    username="mirinae"
	    ;;
	3)
	    service="N:Vnet"
	    targethostname="vnet.postech.ac.kr"
	    protocol="T"
	    username="vnet"
	    ;;
	4)
	    service="N:ctrl-d"
	    targethostname="ctrl-d.postech.ac.kr"
	    protocol="T"
	    username="bbs"
	    ;;
#	5)
#	    service="N:정웅비"
#	    targethostip="141.223.206.34"
#	    protocol="T"
#	    username=""
#	    ;;   
	6)
	    service="N:고답"
	    targethostname="mdom15103-1.postech.ac.kr"
	    protocol="T"
	    username="bbs"
	    ;;
	7)
	    service="N:Green"
	    targethostname="green.postech.ac.kr"
	    protocol="T"
	    username="bbs"
	    ;;
	8)
	    service="N:shell"
	    targethostname="shell.postech.ac.kr"
	    protocol="T"
	    username="-_-?"
	    ;;
	9)
	    service="N:Jbb"
	    targethostip="141.223.201.135"
	    protocol="T"
	    username="bbs"
    	    ;;
	10)
	    service="N:lion"
	    targethostname="lion.postech.ac.kr"
	    protocol="T"
	    username="bbs"
	    ;;
	789)
	    service="삽질중"
	    targethostname="localhost"
	    protocol="R"
	    username="gofeel"
	    ;;

	89)
	    echo
	    echo 접속할 호스트의 IP Address 나 Domain Name 을 입력하세요.  
	    echo
	    echo -n ${SO}telnet ${SE}
	    read HOST
	    service=N:$HOST
	    targethostname=$HOST
	    protocol="T"
	    username=""
	    ;;
	99)
	    service="N:수족관"
	    bin/replace_utmp .UTMP $UTMPENT $service
	    cat bin/animation
	    #scriptfile="bin/fishBread.csh"
	    protocol="N"
	    ;;               
	*)
	    echo ""
	    echo ""
	    echo "${SO}${CHOICE}: 없는 번호입니다. ${SE}"
	    echo -n "<리턴을 눌러주세요>"
	    read KEY
            protocol="N"
	    ;;
    esac

    # 실행 ----------------------------------------------
    echo "$service (아이디 : $USER)" >> bbslog
    case "$protocol" in
	F)  echo -n "$pingtimeout초동안 연결을 시도합니다 "
	    echo -n "($username@$targethostname)... "
	    targethost=$targethostname 
	    ;;
	M)  echo -n "$pingtimeout초동안 연결을 시도합니다 "
	    if [ $?targethostname ]; then
		echo -n "($targethostname "
		targethost=$targethostname
	    else
		echo -n "($targethostip "
		targethost=$targethostip
	    fi
	    echo -n "/ 포트$targetport)... " 
	    ;;
	T)  echo -n "$pingtimeout초동안 연결을 시도합니다 ($username@"
	    case $targethostname in
	    	"") echo -n "$targethostip)... " 
	    	    targethost=$targethostip ;;
	    	*)  echo -n "$targethostname)... " 
	    	    targethost=$targethostname ;;
	    esac 
	    ;;
	R)  echo -n "$pingtimeout초동안 연결을 시도합니다 ($username@"
	    case $targethostname in
	    	"") echo -n "$targethostip)... " 
	    	    targethost=$targethostip ;;
	    	*)  echo -n "$targethostname)... " 
	    	    targethost=$targethostname ;;
	    esac 
	    #echo -n "$targethostname)... "
	    #targethost=$targethostname
	    ;;
    esac

    case $protocol in
	S)  echo "스크립트를 실행합니다. ($scriptfile)"
	    bin/replace_utmp .UTMP $UTMPENT $service ; 
            $scriptfile $1
	    ;;
        N)  echo "선택화면으로 돌아갑니다."
            ;;
	*)
	bin/replace_utmp .UTMP $UTMPENT $service ; 
	case "$protocol" in 
	    "R")  
		( echo "";
		  ${RLOGIN} $targethost -l $username ) ;; 
	    "T") 
		( echo "로그인네임은 ($username)입니다"; 
		  echo ""; 
		  echo "";
		  ${TELNET} $targethost ) 
		  ;;
	esac ;
        echo "접속끝..."; echo "선택화면으로 돌아갑니다";;
    esac;

bin/replace_utmp .UTMP $UTMPENT 나들이 

else
    exit
fi
done

$CLEAR
