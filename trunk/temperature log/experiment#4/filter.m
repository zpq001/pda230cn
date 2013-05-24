
% Data sample time = 100ms

% ADC 
file = fopen('adc.txt','r');
if file ~= -1
    ADC_data = fscanf(file, '%d');
    fclose(file);
end

plot(ADC_data,'r');
grid on;
hold on;
title('Filtered ADC');

%------------------------------------------%
% Filter by built-in function
% a - denominator, b - nominator

% moving average
a = [1];
b = [0.25 0.25 0.25 0.25];  


a = [1 0.2 0.3 0.3 0.1];
b = [2 3];  

Fdata = filter(b,a,ADC_data);


% filter variant 1
% K = [0.2, 0.3, 0.5];
% N = 3;
% for i = N:1:length(ADC_data)
%    fs = 0;
%    for j = 1 : N
%         fs = fs + ADC_data(i-(N-j)) * K(j);
%    end
%    Fdata(i) = fs;
% end

plot(Fdata);
