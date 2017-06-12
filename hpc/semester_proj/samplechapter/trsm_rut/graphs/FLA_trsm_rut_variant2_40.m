% number of repeats:5

figure

% enter mfirst, mlast, minc:100 1000 100
% enter n: (-1 means n = m)-1
% we will take m=n in these experiments
% enter nb_alg:40
% enter variant: (1 == variant 1, 2 == variant 2)2
% Performance in MFLOPS/sec (1e6 flops per second)

%  m    n  nb    REF FLAME   diff      unb      diff    blk     diff   rec    diff
% ==================================================================================
variant2 = [
 100  100  40  193.7  228.5  7.8e-16  104.2  1.0e-15  190.9  7.8e-16  234.9  7.8e-16
 200  200  40  333.7  293.3  8.9e-16   97.4  1.8e-15  258.2  6.7e-16  334.1  8.9e-16
 300  300  40  349.3  332.5  1.0e-15   65.6  3.2e-15  294.5  7.8e-16  352.7  1.1e-15
 400  400  40  361.8  345.5  8.9e-16   52.9  3.0e-15  309.8  8.9e-16  358.0  8.9e-16
 500  500  40  364.7  358.7  8.9e-16   48.6  2.7e-15  326.2  8.9e-16  366.7  7.8e-16
 600  600  40  368.8  367.3  7.8e-16    0.0  0.0e+00  327.9  1.4e-15  367.2  1.0e-15
 700  700  40  373.6  370.7  1.3e-15    0.0  0.0e+00  331.4  1.1e-15  369.0  1.2e-15
 800  800  40  382.7  376.5  1.0e-15    0.0  0.0e+00  334.7  8.9e-16  370.2  8.9e-16
 900  900  40  385.9  378.0  1.2e-15    0.0  0.0e+00  337.8  1.1e-15  367.9  1.1e-15
1000 1000  40  393.1  383.6  8.9e-16    0.0  0.0e+00  341.4  7.2e-16  369.2  6.7e-16
]
plot( variant2( :, 1 ), variant2( :, 4 ), '-', ... 
      variant2( :, 1 ), variant2( :, 5 ), '-.', ... 
      variant2( 1:5, 1 ), variant2( 1:5, 7 ), '--x', ... 
      variant2( :, 1 ), variant2( :, 9 ), '--o', ... 
      variant2( :, 1 ), variant2( :, 11 ), '--+' )
legend( 'Reference', 'FLAME', 'Unblocked', 'Blocked', 'Recursive', 1 )
axis( [0, 1000, 0, 650 ] )
grid on
title( 'B <- B U^-T (variant 2) nb = 40' )
xlabel( 'm = n' )
ylabel( 'MFLOPS/sec.' )
print -depsc2 trsm_rut_variant2_40.eps
