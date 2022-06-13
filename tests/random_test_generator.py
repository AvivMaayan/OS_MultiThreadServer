# to be written
# young grasshopper
from random import seed
from random import randint
import os


def index_command(i):  # returns "index$i.html"
    res = "index"
    res += i
    res += ".html"
    return res

# returns randomly 1,2,3.
# 1 for html, 2 for sleep, 3 for doesn't exist
def command_type():
    return randint(1, 3)


# random sleepy times for our baby server
# i love cupcakes 
def random_sleep():
    return randint(1, 20)

# generates <num_of_comm> random commands to a file called test_me.txt
sleep_comm = " output.cgi?"
if(os.path.isfile("test_me.txt")):
    f = os.open("test_me.txt", os.O_RDWR | os.O_TRUNC)

else :
    f = os.open("test_me.txt", os.O_RDWR | os.O_CREAT)

num_of_comm = 100

final = ""
for i in list(range(num_of_comm)):
    command = str(command_type())
    final = ""
    if command == '1':  # html
        final += " index"
        final += str(command_type())
        final += ".html"

    elif command == '2':  # sleep
        final += sleep_comm
        final += str(random_sleep())

    else:
        final += " doesn't_exist"
    final_bytes = str.encode(final)
    os.write(f, final_bytes)
    os.write(f, str.encode("\n"))

os.close(f)
