# 1. 注册事件
**<mark>server.c : main() --->  initServer() --->  aeCreateFileEvent()</mark>** <br>

<img width="721" height="154" alt="image" src="https://github.com/user-attachments/assets/3c46794a-e7d1-487c-8d8f-0d7e0dc97172" />

<br>

---> **<mark>aeApiAddEvent()</mark>**

<img width="485" height="46" alt="image" src="https://github.com/user-attachments/assets/8e88d7b8-9c7a-4c88-8aa7-055e6c015e54" />

<br>

**<mark>ae_poll.c : aeApiAddEvent()</mark>**

<img width="649" height="326" alt="image" src="https://github.com/user-attachments/assets/c2391e4c-ed8e-4dd4-a65b-194f8c1bafa0" />
