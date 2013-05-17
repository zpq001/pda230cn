
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

% ADC 
file = fopen('adc.txt','r');
if file ~= -1
    ADC_data = fscanf(file, '%d');
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

%--------------------------
% Derivative calculus

Nw = 50;
filtered = (1:length(ADC_data));
for  i = 1:1:length(ADC_data)
    if (i <= Nw)
        filtered(i) = ADC_data(1) * Nw;
    else
        filtered(i) = 0;
        for k = i-Nw : i-1
            filtered(i) = filtered(i) + ADC_data(k);
        end
    end
end
filtered_scaled = filtered / Nw;
hold on;
plot(filtered_scaled,'r');











