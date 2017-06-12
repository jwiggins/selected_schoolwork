% number of repeats:5

figure

% enter mfirst, mlast, minc:100 1000 100
% enter n: (-1 means n = m)-1
% we will take m=n in these experiments
% enter nb_alg:80
% enter variant: (1 == variant 1, 2 == variant 2)1
% Performance in MFLOPS/sec (1e6 flops per second)

%  m    n  nb    REF FLAME   diff      unb      diff    blk     diff   rec    diff
% ==================================================================================
variant1 = [
 100  100  80  192.8  228.1  7.8e-16  221.8  1.3e-15  216.4  6.7e-16  218.5  7.8e-16
 200  200  80  333.9  294.5  8.9e-16  233.2  1.3e-15  311.0  8.9e-16  316.2  8.9e-16
 300  300  80  351.5  332.8  1.0e-15  171.3  1.9e-15  342.4  1.3e-15  344.0  1.1e-15
 400  400  80  359.8  344.9  8.9e-16  135.4  1.8e-15  349.3  1.2e-15  360.2  8.9e-16
 500  500  80  363.6  357.7  8.9e-16  128.0  2.2e-15  345.7  7.2e-16  361.7  7.8e-16
 600  600  80  368.9  366.5  7.8e-16    0.0  0.0e+00  350.9  1.1e-15  369.8  1.0e-15
 700  700  80  373.6  370.6  1.3e-15    0.0  0.0e+00  354.3  1.6e-15  375.9  1.2e-15
 800  800  80  383.1  376.7  1.0e-15    0.0  0.0e+00  356.9  1.2e-15  379.5  8.9e-16
 900  900  80  386.2  378.0  1.2e-15    0.0  0.0e+00  352.3  1.0e-15  376.7  8.9e-16
1000 1000  80  393.8  382.7  8.9e-16    0.0  0.0e+00  352.9  9.4e-16  381.6  7.8e-16
]
plot( variant1( :, 1 ), variant1( :, 4 ), '-', ... 
      variant1( :, 1 ), variant1( :, 5 ), '-.', ... 
      variant1( 1:5, 1 ), variant1( 1:5, 7 ), '--x', ... 
      variant1( :, 1 ), variant1( :, 9 ), '--o', ... 
      variant1( :, 1 ), variant1( :, 11 ), '--+' )
legend( 'Reference', 'FLAME', 'Unblocked', 'Blocked', 'Recursive', 1 )
axis( [0, 1000, 0, 650 ] )
grid on
title( 'B <- B U^-T (variant 1) nb = 80' )
xlabel( 'm = n' )
ylabel( 'MFLOPS/sec.' )
print -depsc2 trsm_rut_variant1_80.eps
