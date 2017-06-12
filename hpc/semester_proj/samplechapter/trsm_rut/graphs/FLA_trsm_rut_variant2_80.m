% number of repeats:5

figure

% enter mfirst, mlast, minc:100 1000 100
% enter n: (-1 means n = m)-1
% we will take m=n in these experiments
% enter nb_alg:80
% enter variant: (1 == variant 1, 2 == variant 2)2
% Performance in MFLOPS/sec (1e6 flops per second)

%  m    n  nb    REF FLAME   diff      unb      diff    blk     diff   rec    diff
% ==================================================================================
variant2 = [
 100  100  80  193.7  229.4  7.8e-16  104.3  1.0e-15  132.1  7.8e-16  228.2  7.8e-16
 200  200  80  335.4  295.2  8.9e-16   88.3  1.8e-15  199.2  7.8e-16  325.3  8.9e-16
 300  300  80  350.1  333.0  1.0e-15   64.9  3.2e-15  233.1  1.1e-15  350.3  1.1e-15
 400  400  80  362.1  347.0  8.9e-16   52.7  3.0e-15  253.4  1.2e-15  364.1  8.9e-16
 500  500  80  365.2  358.9  8.9e-16   48.6  2.7e-15  277.4  9.4e-16  372.7  7.8e-16
 600  600  80  369.1  367.2  7.8e-16    0.0  0.0e+00  273.9  1.4e-15  377.0  1.0e-15
 700  700  80  373.4  370.3  1.3e-15    0.0  0.0e+00  276.4  1.7e-15  376.8  1.2e-15
 800  800  80  382.4  376.8  1.0e-15    0.0  0.0e+00  285.5  1.4e-15  380.0  8.9e-16
 900  900  80  385.5  377.1  1.2e-15    0.0  0.0e+00  287.0  1.1e-15  377.9  1.1e-15
1000 1000  80  392.8  382.7  8.9e-16    0.0  0.0e+00  291.1  8.9e-16  381.0  7.8e-16
]
plot( variant2( :, 1 ), variant2( :, 4 ), '-', ... 
      variant2( :, 1 ), variant2( :, 5 ), '-.', ... 
      variant2( 1:5, 1 ), variant2( 1:5, 7 ), '--x', ... 
      variant2( :, 1 ), variant2( :, 9 ), '--o', ... 
      variant2( :, 1 ), variant2( :, 11 ), '--+' )
legend( 'Reference', 'FLAME', 'Unblocked', 'Blocked', 'Recursive', 1 )
axis( [0, 1000, 0, 650 ] )
grid on
title( 'B <- B U^-T (variant 2) nb = 80' )
xlabel( 'm = n' )
ylabel( 'MFLOPS/sec.' )
print -depsc2 trsm_rut_variant2_80.eps
