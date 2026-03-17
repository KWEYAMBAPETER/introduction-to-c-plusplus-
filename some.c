#include <stdio.h>
#include <math.h>
int main(){
    int p,q,r;
    int sum =q+r+p;
    int product = p*q*r;
    int division = p*q/r;
    printf("Enter the value of P: \n ");
    scanf("%d", &p);
    printf("Enter the value of Q: \n ");
    scanf("%d", &q);
    printf("Enter the value of R: \n ");
    scanf("%d", &r);

    printf("The sum of p, q and r is %d", q+p+r);
    printf("\n The product of p, q and r is %d", q*p*r);
    printf("\n The division of p, q and r is %d", q*p/r);
  

    return 0;
};