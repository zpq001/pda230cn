

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

%------------------

file = fopen('f2_adc.txt','r');
if file ~= -1
    f2_adc = fscanf(file, '%f');
    fclose(file);
end

file = fopen('f2_celsius.txt','r');
if file ~= -1
    f2_celsius = fscanf(file, '%d');
    fclose(file);
end


plot(f1_adc,f1_celsius,'r');
hold on;

plot(f2_adc,f2_celsius,'b');
hold on;



grid on;
title('ADC - Celsius scale');
%xlabel('x 100ms');
%ylabel('degrees Celsius');
%legend('Actual','Desired','PID effort');










