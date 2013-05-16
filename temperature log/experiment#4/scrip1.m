
% Actual temperature
file = fopen('celsius.txt','r');
if file ~= -1
    temp_actual = fscanf(file, '%d');
    fclose(file);
end

% Desired temperature
file = fopen('setting.txt','r');
if file ~= -1
    temp_set = fscanf(file, '%d');
    fclose(file);
end

% PID output
file = fopen('control.txt','r');
if file ~= -1
    PID_effort = fscanf(file, '%d');
    PID_effort = PID_effort * 10;
    fclose(file);
end

plot(temp_actual,'r');
hold on;
plot(temp_set,'b');
hold on;
plot(PID_effort,'m');
grid on;
title('Temperature setting');
xlabel('ms');
ylabel('degrees Celsius');
legend('Actual','Desired','PID effort');



% file = fopen('setting.txt','w');
% if file ~= -1
%     a = 0;
%     while a < 12446
%        fprintf(file,'%d\r',120);
%        a = a + 1;
%     end
%     while a < 22840
%        fprintf(file,'%d\r',150);
%        a = a + 1;
%     end
%     while a < 36673
%        fprintf(file,'%d\r',0);
%        a = a + 1;
%     end
%     fclose(file);
% end



