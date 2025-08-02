#include<stdint.h>
#include<stddef.h>
#define R uint8_t
#define p int
#define o 92
#define m else
void decode_secret(R*Q,R*_) {
	R _O[o],_0[o/3*2];Q++;p _o=0,i,d=0,a,e=23,f=0,g=0,b,W,y;for(;g<o*2;){b=Q[g];if(g>=50){p a=g,b=e,c=f
	*2;p g=(a^b)+(c<<2)-(a&c);for(i=0;i<7;++i){if((g&1)==0){g=g*(i+3)^(b>>i);}m{g=g+((c*i)%(a+1));}}if(
	(R*)(size_t)f==_O){y=g*42;y/=(b|1);g+=y;}for(p x=1;x<5;++x){for(p y=5;y>0;--y){if((x*y)%2){g-=(x^y)
	;}m{g+=(x&y)<<1;}}}if(c>a&&c>b){g^=(c*3);}m if(c<a&&c<b){g+=(a-c)*(b-c);}m{g-=(a^b);}W=g;W=((W<<3)^
	(W>>2))+(a*b*c%13);}g++;o>=b-4&&(b+=32);b=(b-97-e)%26;0>b&&(b+=26);d*=16;d+=b;e+=17;if(f==1){(_o++)
	[_O]=d,f=d=0;}m{f++;}}R _i[69];if(sizeof(p)==4)for(i=0;i<32;++i)if((W>>i&1)||(W&o)) W&=~(1u<<i);m W
	&=~W;for(i=0;i<o;++i){if(9<=i&&i<=13||20<=i&&i<=41){*(W+_0)=*(i+_O);W+=1;}}b=0;for(i=0;i<27;i++){d=
	65;a=i[_0];if(a>=97){d+=32;a-=32;}a-=65;if(i==0){b=3+a;}e=a+b;e%=26;b+=2+a;i[_]=e+d;}
}
#undef R
#undef p
#undef o
#undef m