
#!/bin/bash

program="LogonServer"

ps -ef | grep $program | cut -c 9-15 | xargs kill -9

eStr=${program}" is stop!!!"

echo $eStr

