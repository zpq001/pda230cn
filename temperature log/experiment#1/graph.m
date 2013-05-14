

ref_data = (0);
timestamp = (0);
sensor_data = (0);
control = (0);

file = fopen('uart_log.txt','r');
if file ~= -1
    str = fgetl(file);
    k = 1;
    while(str ~= -1)
       temp = sscanf(str, '%d %d %d', [1, inf]);
       timestamp(k) = temp(1);
       sensor_data(k) = temp(2);
       control(k) = temp(3);
       k = k + 1;
       str = fgetl(file);
    end
    fclose(file);
end

file = fopen('UT71_log.txt','r');
if file ~= -1
    str = fgetl(file);
    k = 1;
    while(str ~= -1)
       str = strrep(str,',','.');
       ref_data(k) =  sscanf(str, '%f');
       k = k + 1;
       str = fgetl(file);
    end
    fclose(file);
end

%-------------------------------------------
% Calculate offset and coefficient for
% diode sensor
d_max = min(sensor_data);       % the most hot point from sensor
d_min = max(sensor_data);       % the most cold point from sensor
ref_max = max(ref_data);        % the most hot reference point 
ref_min = min(ref_data);        % the most cold reference point 
A = [d_max 1
     d_min 1];
B = [ref_max; ref_min];
X = A\B;
k_norm = X(1);
offset_norm = X(2);
%-------------------------------------------

    % add elements to the reference vector
    prehist_length = 71;
    ref_data_prehistory = repmat(ref_data(1),1,prehist_length);
    ref_data = [ref_data_prehistory, ref_data];
    
    sensor_data_norm = sensor_data * k_norm + offset_norm;
    
    plot(timestamp(1:3000),sensor_data_norm(1:3000),'b');
    hold all
    plot(timestamp(1:3000),ref_data(1:3000),'r');
    hold all
    plot(timestamp(1:3000),control(1:3000)*150,'g');
    
    grid on;
    title('Temperature from diode sensor vs UT71C');
    xlabel('Seconds');
    ylabel('degrees Celsius');
    legend('Diode sensor, normalized','UT71C','Heater ON/OFF');
    
%-------------------------------------------
%[num,den] = butter(5, .1 ) ;
%sensor_data_filtered = filter(num,den,sensor_data_norm);
%plot(timestamp(1:3000),sensor_data_filtered(1:3000),'m');

z = iddata(sensor_data_norm(1:3000)',control(1:3000)',1);
%z = iddata(sensor_data(1:3000)',control(1:3000)',1);
nb=[3]; % Model order
nf=[3]; % Model order
nk=[1]; 

% Идентификация 
we=oe(z,[nb nf nk]); 
wet=tf(d2c(we)); 
wo=wet(1)
XOest=findstates(we,z); 
sim_reaction = sim(we,control','InitialState',XOest);

hold all
plot(timestamp(1:3000),sim_reaction(1:3000),'m');

%c = pidtune(wo,'pid');
%Tpi = feedback(c*wo, 1);
%figure();
%step(Tpi)

    
    