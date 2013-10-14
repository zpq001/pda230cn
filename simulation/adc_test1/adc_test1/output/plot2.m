

file = fopen('f2_adc.txt','r');
if file ~= -1
    f2_adc = fscanf(file, '%f');
    fclose(file);
end

file = fopen('f2_adc_float.txt','r');
if file ~= -1
    f2_adc_float = fscanf(file, '%f');
    fclose(file);
end

file = fopen('f2_celsius.txt','r');
if file ~= -1
    f2_celsius = fscanf(file, '%d');
    fclose(file);
end



plot(f2_celsius,f2_adc,'r');
hold on;

plot(f2_celsius,f2_adc_float,'b');
hold on;

grid on;
title('ADC - Celsius scale');
%xlabel('x 100ms');
%ylabel('degrees Celsius');
%legend('Actual','Desired','PID effort');










