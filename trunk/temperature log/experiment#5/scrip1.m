
% Actual temperature
file = fopen('celsius.txt','r');
if file ~= -1
    temp_actual = fscanf(file, '%d');
    fclose(file);
end


plot(temp_actual,'r');
hold on;

grid on;
title('Temperature setting');
xlabel('x 100ms');
ylabel('degrees Celsius');
legend('Actual','Desired','PID effort');










