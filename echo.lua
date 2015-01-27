--audio echo effect
echoWidth = 100
echoDepth = 0.01
for index=echoWidth,dataLen-1,1 do
	data[index] = data[index] + echoDepth*data[index-echoWidth]
end