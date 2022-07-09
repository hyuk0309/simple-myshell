# Simple MyShell

## shell의 동작 원리

shell의 동작 원리는 shell process에서 fork()를 통해 프로세스를 만들고, 자식 프로세스에서 exec()를 통해 입력 받은 명렁어를 수행한다. 그리고 부모 프로세스인 shell은 wait()를 이용해 자식의 작업을 기다린다.

## 구현 기능

### cd command
일반적인 방식으로 cd 명령어를 수행하면, 자식의 위치가 변하기 때문에 cd 명령어는 shell 프로세스에서 바로 수행해야 한다. 

*코드*

![cd](https://user-images.githubusercontent.com/29492667/177816402-9fc3700e-9552-4942-9cc9-0fa7aaa3289d.png)
![cd2](https://user-images.githubusercontent.com/29492667/177817035-3d6726c5-9295-4be8-a600-f3174e766a87.png)

*테스트*

![cd-test](https://user-images.githubusercontent.com/29492667/177817066-09a2811e-8594-4e50-a695-4a700905f9e3.png)

### exit commmand
exit 명령어를 입력 받으면 자식 프로세스를 생성하기 전에 shell 프로세스를 종료한다.

*코드*

![exit](https://user-images.githubusercontent.com/29492667/177817806-2f1f9b21-ad94-495d-9e5a-2878b9feb026.png)

*테스트*

![exit-test](https://user-images.githubusercontent.com/29492667/177817819-57c0dfbf-8324-4e3b-b168-2c04ebbed492.png)

### & command (background)
shell의 자식 프로세스가 입력 받은 명령어를 수행하지 않고, shell의 자식 프로세스의 자식 프로세스가 명령을 실행한다.
shell의 자식 프로세스는 자식을 만들고 죽는다. 즉, 실제 명령을 수행하는 프로세스는 고아 프로세스다.

*코드*

![background](https://user-images.githubusercontent.com/29492667/177819512-d0f484a0-0726-41a3-945a-dd247b7ab2a7.png)

*테스트*

![background-test](https://user-images.githubusercontent.com/29492667/177819524-07e59191-c3d0-4c37-8893-57cc020e1449.png)

### SIGCHLD
background 실행을 위한 고아 프로세스를 만드는 과정에서 shell의 자식이 죽으면 자식은 좀비 프로세스가 된다.

이를 해결하기위해 SIGCHLD를 처리하는 signal handler 등록했다. 
또한 fgets()로 사용자의 입력을 기다리고 있는 상황에서 SIGCHLD 시그널이 처리되면 fgets()는 에러를 반환하기 때문에 goto를 이용해 다시 입력을 기다리도록 처리했다.

*코드*

![sigchld-1](https://user-images.githubusercontent.com/29492667/178111864-2ceb40ab-c4ac-42f2-83af-a9405380bf84.png)
![sigchld-2](https://user-images.githubusercontent.com/29492667/178111882-e0a2477e-fe48-4d58-9fcc-d8564609de14.png)
![sigchld-3](https://user-images.githubusercontent.com/29492667/178111887-8e1ab047-c726-451a-a7fd-b878b32eaced.png)

*테스트*

![sigchld-test-1](https://user-images.githubusercontent.com/29492667/178111923-01382632-3ca2-43e3-9186-e3783dee8d3d.png)
![sigchld-test-2](https://user-images.githubusercontent.com/29492667/178111926-8c4a8f21-fdce-4ab2-a3c0-86be923e78ba.png)

### SIGINT, SIGQUIT

shell에서는 SIGINT(Ctrl+C), SIGQUIT(Ctrl+\\)는 무시해야 하고, foreground 프로세스(shell의 자식 프로세스)에서는 default로 처리해야 한다.
따라서 shell에서는 SIGINT, SIGQUIT을 무시하고, 자식 프로세스에서는 default로 처리했다.

*코드*

![signal-1](https://user-images.githubusercontent.com/29492667/178112126-99d6d539-68f2-46ad-bc8f-adab39f315aa.png)
![signal-2](https://user-images.githubusercontent.com/29492667/178112128-6a3590fe-daf2-422f-855f-47ea8cb9bd1f.png)

*테스트*

![signal-test-1](https://user-images.githubusercontent.com/29492667/178112157-8b2f3f0a-7791-4a0b-988e-31eb9b880b9f.png)
![signal-test-2](https://user-images.githubusercontent.com/29492667/178112158-f1f0bab5-4767-4d49-abe6-ea7c649a0a90.png)

### redirection command

명령에 redirection 관련 명령어가 있는지 확인한다. 
명령어가 있으면 명령어에 맞게 표준 입출력을 해당 파일로 바꿔준다.

*코드*

![redirection-1](https://user-images.githubusercontent.com/29492667/178113135-3d21e635-4629-4117-b937-5f7480990bb5.png)
![redirection-2](https://user-images.githubusercontent.com/29492667/178113136-184ea1de-ba53-4ce2-bebd-42efbec0f4f4.png)

*테스트*

![redirection-test](https://user-images.githubusercontent.com/29492667/178113158-ca3d658f-0552-4523-b108-433bcc1259ba.png)

### pipe command

명령에 pipe 관련 명령어가 있는지 확인한다.
shell의 자식 프로세스에서 자식 프로세스를 만들어 표준 출력을 pipe 바꿔주고, shell의 자식 프로세스의 표준 입력도 pipe로 바꿔준다. 
마지막은 fork를 이용하지 않고, shell의 자식 프로세스에서 수행한다.

*코드*

![pipe-1](https://user-images.githubusercontent.com/29492667/178113644-40dfe7ca-388c-460b-938f-20d12a253d49.png)
![pipe-2](https://user-images.githubusercontent.com/29492667/178113645-c8de86e2-f115-44bc-9cdd-1b54676b10c3.png)

*테스트*

![pipe-test](https://user-images.githubusercontent.com/29492667/178113647-b6f7c640-c384-4d3d-9e15-e6ea56ce3e13.png)

### redirection pipe in background

고아 프로세스에서 redirection, pipe 명령을 수행할 수 있도록한다.

*코드*

<img width="452" alt="redirection-pipe-background" src="https://user-images.githubusercontent.com/29492667/178113785-14279708-d3a8-498d-b616-34a738ce90b1.png">

*테스트*

![redirection-pipe-background-test](https://user-images.githubusercontent.com/29492667/178113754-88ffddb9-617c-4334-9a89-9a591b250f31.png)
