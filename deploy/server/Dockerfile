FROM ubuntu:16.04

RUN apt-get update && apt-get -y upgrade

ENV PROB chat_server

RUN useradd -m $PROB

WORKDIR /home/$PROB

COPY $PROB /home/$PROB/$PROB

RUN chown -R root:$PROB /home/$PROB
RUN chmod 750 /home/$PROB
RUN chmod 750 /home/$PROB/$PROB

# Something run script
