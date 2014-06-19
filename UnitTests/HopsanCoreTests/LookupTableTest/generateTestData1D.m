

% Test data for 1D lookup

ts = 0.001;

t=linspace(1,10,1/ts);
y=sin(2*pi*t);

% Save data to CSV

t_ds = downsample(t, 10);
y_ds = downsample(y, 10);

figure()
hold on
plot(t,y, 'b')
plot(t_ds, y_ds, 'r-*')
hold off

dlmwrite('test1d.csv', [t_ds; y_ds]');
