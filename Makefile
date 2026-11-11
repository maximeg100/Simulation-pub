all:
	mkdir -p logs
	gcc -Wall -Wextra -pthread -Icommon -Iserver server/main_pub.c server/client_handler.c server/tables.c server/waiter.c server/barman.c common/logger.c -o pub
	gcc -Wall -Wextra -pthread -Icommon client/main_client.c common/logger.c -o client

clean:
	rm -f pub client logs/*.log