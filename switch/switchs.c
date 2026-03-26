#include <stdio.h>
int main(){
// int choice =2;
// switch (choice){
//     case1:
//     printf("eyeye");
//     break;
//     case2:
//     printf("and this heheh");
//     break;

//     default:
//     printf("and yes");

// }




int i;
printf("Enter the value of 'i' for 0 <= i >= 2:\n");
scanf("%d",&i);
while(i<10)
{
    printf("the numbers are %d\n",i)
    ;
i = i+2;
}

printf("This is the For LOOP in Progress.\n");
for(int i = 0 ;i<17; i++)
printf("The number is %d\n",i);


printf("This is the Do while LOOP!!\n");

do{
    printf("the num is %d\n", i);
    i =+ 1;
}
while(i>0 && i<16);
   return 0;
}