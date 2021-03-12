ans = list()
with open('./benchmark', 'r') as file:
    line = file.readline().split(' ', 1)
    for _ in range(5):
        temp = 0
        for _ in range(5):
            temp += float(line[0])
            line = file.readline().split(' ', 1)  
        ans.append(temp/5)

print(ans)