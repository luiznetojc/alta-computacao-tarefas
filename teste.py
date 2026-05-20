valores = [1, 1, 2, 1, 4, 1, 2, 2] # 8 elementos, majoritario seria n/2 = 4
frequencia = 0
atual = 0
for i in range(0, len(valores)):
	if frequencia == 0:
		atual = valores[i]
		frequencia += 1
	
	elif atual == valores[i]:
		frequencia += 1
	
	else:
		frequencia -=1
print(atual)
