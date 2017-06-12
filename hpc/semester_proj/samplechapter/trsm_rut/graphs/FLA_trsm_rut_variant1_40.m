% number of repeats:5

figure

% enter mfirst, mlast, minc:100 1000 100
% enter n: (-1 means n = m)-1
% we will take m=n in these experiments
% enter nb_alg:40
% enter variant: (1 == variant 1, 2 == variant 2)1
% Performance in MFLOPS/sec (1e6 flops per second)

%  m    n  nb    REF FLAME   diff      unb      diff    blk     diff   rec    diff
% ==================================================================================
variant1 = [
 100  100  40  192.6  228.9  7.8e-16  220.3  1.3e-15  225.1  6.7e-16  219.1  7.8e-16
 200  200  40  331.4  293.2  8.9e-16  285.7  1.3e-15  312.8  8.9e-16  311.5  8.9e-16
 300  300  40  349.9  332.8  1.0e-15  164.1  1.9e-15  329.6  7.8e-16  324.9  1.1e-15
 400  400  40  360.3  345.9  8.9e-16  134.0  1.8e-15  344.3  1.0e-15  341.6  8.9e-16
 500  500  40  364.1  357.6  8.9e-16  127.9  2.2e-15  346.6  6.7e-16  345.6  7.8e-16
 600  600  40  368.5  366.5  7.8e-16    0.0  0.0e+00  352.6  1.1e-15  351.2  1.0e-15
 700  700  40  372.4  369.6  1.3e-15    0.0  0.0e+00  348.1  8.9e-16  348.9  8.9e-16
 800  800  40  379.7  375.1  1.0e-15    0.0  0.0e+00  349.4  1.3e-15  351.9  1.0e-15
 900  900  40  385.5  378.0  1.2e-15    0.0  0.0e+00  342.9  1.0e-15  343.5  8.9e-16
1000 1000  40  393.5  383.1  8.9e-16    0.0  0.0e+00  340.9  9.2e-16  345.5  8.6e-16
]
plot( variant1( :, 1 ), variant1( :, 4 ), '-', ... 
      variant1( :, 1 ), variant1( :, 5 ), '-.', ... 
      variant1( 1:5, 1 ), variant1( 1:5, 7 ), '--x', ... 
      variant1( :, 1 ), variant1( :, 9 ), '--o', ... 
      variant1( :, 1 ), variant1( :, 11 ), '--+' )
legend( 'Reference', 'FLAME', 'Unblocked', 'Blocked', 'Recursive', 1 )
axis( [0, 1000, 0, 650 ] )
grid on
title( 'B <- B U^-T (variant 1) nb = 40' )
xlabel( 'm = n' )
ylabel( 'MFLOPS/sec.' )
print -depsc2 trsm_rut_variant1_40.eps
