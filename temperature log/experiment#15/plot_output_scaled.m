
% Actual temperature
file = fopen('col_0.txt','r');
if file ~= -1
    temp_actual = fscanf(file, '%d');
    fclose(file);
end

% P-term
file = fopen('col_5.txt','r');
if file ~= -1
    p_term = fscanf(file, '%d');
    fclose(file);
end

% D-term
file = fopen('col_6.txt','r');
if file ~= -1
    d_term = fscanf(file, '%d');
    fclose(file);
end

% I-term
file = fopen('col_7.txt','r');
if file ~= -1
    i_term = fscanf(file, '%d');
    fclose(file);
end

% Output
file = fopen('col_8.txt','r');
if file ~= -1
    pid_output = fscanf(file, '%d');
    fclose(file);
end


plot(temp_actual,'r');
hold on;

plot(pid_output/5,'b');
hold on;

plot(i_term/99,'c');
hold on;

plot(d_term/99,'m');
hold on;

plot(p_term/99,'y');
hold on;

grid on;
title('Temperature setting');
xlabel('x 100ms');
%ylabel('degrees Celsius');
%legend('Actual','Desired','PID effort');










