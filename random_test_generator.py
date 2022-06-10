# to be written 
# young grasshopper
from random import seed
from random import randint

def index_command(i): #returns "index$i.html"
    res = "index"
    res += i
    res += ".html"
    return res

# returns randomly 1,2,3. 
# 1 for html, 2 for sleep, 3 for doesn't exist
def command_type():
    seed(1)
    return randint(0,3)

def random_sleep():
    seed(1)
    return randint(0,20)

#example: ./main 30 2010 
#generates 30 random commands for port 2010
def main(comm, argv):
    first_half = "./client localhost "
    sleep_comm = " output.cgi?"
    f = open("test_file.txt", 'rw')
    if(comm > 0):
        num_of_comm = comm
        port = argv
    else:
        num_of_comm = 30

    for i in list(range(1,num_of_comm)):
        command = command_type()   
        final = first_half 
        final += port 
        if command == 1: #html
            final += " index"
            final += command_type()
            final += ".html"

        elif command == 2: #sleep
            final += sleep_comm
            final += random_sleep()
            
        else:
            final += " doesn't exist"

        f.write(final)
        f.write("\n")
