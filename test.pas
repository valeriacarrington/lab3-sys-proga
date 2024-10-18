program  n_3_1;
  var s, i: integer;
      a: array[1..10] of integer;
begin 
  s:=0;
  randomize;

  for i:=1 to 10 do
     begin 
        a[i]:=random(100);
        write(a[i],' ');
        s:=s+a[i]
     end;

  writeln('s=', s)
end.