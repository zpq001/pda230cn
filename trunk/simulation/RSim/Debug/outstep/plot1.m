
% Actual temperature
file = fopen('col_0.txt','r');
if file ~= -1
    temp_simulated = fscanf(file, '%d');
    fclose(file);
end

file = fopen('step_ref.txt','r');
if file ~= -1
    temp_ref = fscanf(file, '%d');
    fclose(file);
end

file = fopen('setting.txt','r');
if file ~= -1
    temp_setting = fscanf(file, '%d');
    fclose(file);
end

plot(temp_simulated,'r');
hold on;

plot(temp_ref,'b');
hold on;

plot(temp_setting,'g');
hold on;

%plot(d_term,'g');
hold on;


grid on;
title('Temperature setting');
xlabel('x 100ms');
ylabel('degrees Celsius');
legend('Simulated','Reference','Heater');










