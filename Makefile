.PHONY:all
all:
	@cd compile_server;\
	make;\
	cd -;\
	cd oj_server;\
	make;\
	cd -;

.PHONY:output
output:all
	@rm -rf output;\
	mkdir -p output/compile_server/temp;\
	mkdir -p output/oj_server/lib;\
	cp -rf compile_server/compile_server output/compile_server/;\
	cp -rf oj_server/conf output/oj_server/;\
	cp -rfL oj_server/lib/libmysqlclient.so* output/oj_server/lib/;\
	cp -rf oj_server/sql output/oj_server/;\
	cp -rf oj_server/template_html output/oj_server/;\
	cp -rf oj_server/wwwroot output/oj_server/;\
	cp -rf oj_server/oj_server output/oj_server/;\
	printf '#!/bin/bash\nexport LD_LIBRARY_PATH=./lib:$$LD_LIBRARY_PATH\n./oj_server\n' > output/oj_server/start.sh;\
	chmod +x output/oj_server/start.sh;

.PHONY:clean
clean:
	@cd compile_server;\
	make clean;\
	cd -;\
	cd oj_server;\
	make clean;\
	cd -;\
	rm -rf output;