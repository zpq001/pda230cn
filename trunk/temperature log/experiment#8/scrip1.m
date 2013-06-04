
% Actual temperature
file = fopen('col_4.txt','r');
if file ~= -1
    temp_actual = fscanf(file, '%d');
    fclose(file);
end

file = fopen('col_10.txt','r');
if file ~= -1
    pid_output = fscanf(file, '%d');
    fclose(file);
end


plot(temp_actual,'r');
hold on;

plot(pid_output * 2,'b');
hold on;

%plot(d_term,'g');
hold on;


grid on;
title('Temperature setting');
xlabel('x 100ms');
%ylabel('degrees Celsius');
%legend('Actual','Desired','PID effort');










