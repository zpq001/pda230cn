

file = fopen('f1_adc.txt','r');
if file ~= -1
    f1_adc = fscanf(file, '%d');
    fclose(file);
end

file = fopen('f1_celsius.txt','r');
if file ~= -1
    f1_celsius = fscanf(file, '%f');
    fclose(file);
end



plot(f1_adc,f1_celsius,'r');
hold on;

grid on;
title('ADC - Celsius scale');
%xlabel('x 100ms');
%ylabel('degrees Celsius');
%legend('Actual','Desired','PID effort');










