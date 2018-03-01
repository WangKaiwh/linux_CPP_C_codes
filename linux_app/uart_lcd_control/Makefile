###########################################  
#Makefile for simple programs  
###########################################  
INC=  
LIB=
  
CC=../../aspeed_arm_gcc
CC_FLAG=-Wall  
  
PRG=uart_lcd
OBJ=uart_raw.o
  
$(PRG):$(OBJ)
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)

.PHONY:clean  
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f $(OBJ) $(PRG)  


 
