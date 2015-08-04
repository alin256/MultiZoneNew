f_in = open('all_locations_from_wiki', 'r')
f_out = open('../src/place_list.h', 'w')
f_out.write("#pragma once \n\n")
f_out.write("const char *places[] = {\n")
line_num = 0;
entry_num = 0;
s = "";
for line in f_in:
    short_line = line.strip();
    if line_num % 8 == 3:
        #do something
        s = short_line.strip(" |[]\n").split("|")[0]
    if line_num % 8 == 7:
        if (short_line.lower().find("link")<0) & (s.find("/")>=0):
            if (entry_num>0):
                f_out.write(",\n")
            #check for link to another one
            f_out.write("\""+s+"\"")
            entry_num += 1
    #print(len(short_line))
    #print(short_line)
    if (len(short_line)>0):
        if (short_line[0]=='|'):
            line_num += 1

f_out.write("};\n")