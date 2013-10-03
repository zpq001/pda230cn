
% Actual temperature
file = fopen('col_0.txt','r');
if file ~= -1
    temp_actual = fscanf(file, '%d');
    fclose(file);
end

file = fopen('col_7.txt','r');
if file ~= -1
    i_term = fscanf(file, '%d');
    fclose(file);
end

file = fopen('col_8.txt','r');
if file ~= -1
    pid_output = fscanf(file, '%d');
    fclose(file);
end

file = fopen('setting.txt','r');
if file ~= -1
    temp_setting = fscanf(file, '%d');
    fclose(file);
end

plot(temp_actual,'r');
hold on;

plot(pid_output,'b');
hold on;

plot(temp_setting,'g');
hold on;

plot(i_term/99,'c');
hold on;


grid on;
title('Temperature setting');
xlabel('x 100ms');
%ylabel('degrees Celsius');
%legend('Actual','Desired','PID effort');










