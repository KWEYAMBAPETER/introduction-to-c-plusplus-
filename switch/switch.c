#include <stdio.h>
#include <string.h>

int main(){
    char Fruit [50];  // Array that can hold up to 49 characters + null terminator
    printf("Kindly enter the type of Fruit you like:\n");
    scanf("%s", Fruit);  // No & needed for array name
    printf("You like %s !", Fruit);
    
    return 0;
}