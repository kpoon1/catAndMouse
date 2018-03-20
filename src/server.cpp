#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <vector>

std::map<sf::IpAddress,std::string> clientList; 
std::map<sf::IpAddress,sf::IpAddress> gameList;

std::map<sf::IpAddress,std::string>::iterator clientIt;



sf::IpAddress findByName(std::string name){
    for(clientIt=clientList.begin(); clientIt!=clientList.end(); clientIt++){
        if(clientIt->second==name)
            break;
    }

    return clientIt->first;
 
}


int main(){
    sf::UdpSocket socket;
    sf::IpAddress remoteIP;
    std::string serverName;
    unsigned short clientPort=45001;
    unsigned short serverPort=45000;
    unsigned short remotePort;

    std::map<sf::IpAddress,bool> clientStillConnect;
    
        
    socket.bind(serverPort);
    socket.setBlocking(false);
    
    std::cout<<"Server Name: ";
    std::cin>>serverName;
    
    std::cout<<"Waiting for client"<<std::endl;
    sf::Clock clock,clock2;
    while(true){
        sf::Packet packet;
        
        //Check if client still connected
        sf::Time t=clock.getElapsedTime();
        if(t.asSeconds()>30){
            std::cout<<"Client List:\n IP\t\tName\n"<<std::endl;
            for(clientIt=clientList.begin();clientIt!=clientList.end();clientIt++){
                std::cout<<clientIt->first<<"\t"<<clientIt->second<<std::endl;
                std::map<sf::IpAddress,sf::IpAddress>::iterator gameIt=gameList.find(clientIt->first);
                if(gameIt==gameList.end())
                	clientStillConnect.insert(std::make_pair(clientIt->first,false));
                
                bool discover=true,chat=false,invite=false,play=false,gameover=false;
                packet<<discover<<chat<<invite<<play<<gameover<<serverName;
                socket.send(packet,clientIt->first,clientPort);
            }
            clock.restart();
            clock2.restart();
        }
        sf::Time t2=clock2.getElapsedTime();
        if(t2.asSeconds()>10){
            std::map<sf::IpAddress,bool>::iterator it;
            for(it=clientStillConnect.begin(); it!=clientStillConnect.end(); it++){
                if(it->second==false){
                    std::cout<<it->first<<" lost connection."<<std::endl;
                	clientList.erase(it->first);

                }
                
            }
            clientStillConnect.clear();
        }
            
        
        if(socket.receive(packet,remoteIP,remotePort)==sf::Socket::Done){
            bool discover,chat,invite,play,gameover;
            std::string msg;
            packet>>discover>>chat>>invite>>play>>gameover>>msg;
            packet.clear();
            if(discover && !chat){
                if (msg.compare("RESPONSE")!=0){
                    std::map<sf::IpAddress,sf::IpAddress>::iterator it;
                    it=gameList.find(remoteIP);
                    if(it!=gameList.end()){ //reconnect system
                        discover=false;
                        play=true;
                        msg=clientList.find(it->second)->second;
                    }else{
                        msg=serverName;
                    }
                    packet<<discover<<chat<<invite<<play<<gameover<<msg;
                    socket.send(packet,remoteIP,clientPort);
                }else{
                    clientStillConnect.find(remoteIP)->second=true;
                    
                        
                }


            }else if(discover && chat){
                if(msg.compare("REQUEST NUM CLIENTS")!=0){
                    clientList.insert(std::make_pair(remoteIP,msg));
                }
                msg=std::to_string(clientList.size());
                packet<<discover<<chat<<invite<<play<<gameover<<msg;
                socket.send(packet,remoteIP,clientPort);
               
            }else if(chat && !invite) {
                std::string playerName=clientList.find(remoteIP)->second;
                std::string bcMsg=playerName+" : "+msg;
                packet<<discover<<chat<<invite<<play<<gameover<<bcMsg;

                for(clientIt=clientList.begin(); clientIt!=clientList.end(); clientIt++){
                    socket.send(packet,clientIt->first,clientPort);
                }
                
            }else if (chat && invite){
                std::string playerName=clientList.find(remoteIP)->second; 
/*
                for(clientIt=clientList.begin(); clientIt!=clientList.end(); clientIt++;){
                    if(clientIt->second==msg)
                        break;
                }
*/
                sf::IpAddress oppIP=findByName(msg);
                packet<<discover<<chat<<invite<<play<<gameover<<playerName;
                socket.send(packet,oppIP,clientPort);
            }else if (invite && play){
                char choice = msg[0];
                msg.erase(msg.begin()); //msg become opponent name
                std::string playerName=clientList.find(remoteIP)->second;

                sf::IpAddress OppIP=findByName(msg);
                                
                if (choice=='y'){
                    gameList.insert(std::make_pair(remoteIP,OppIP));
                    gameList.insert(std::make_pair(OppIP,remoteIP));
                }
                msg=choice+playerName;
                packet<<discover<<chat<<invite<<play<<gameover<<msg;
                socket.send(packet,OppIP,clientPort);                
            
            }else if ((!invite && play)||gameover){
                sf::IpAddress OppIP=gameList.find(remoteIP)->second;
                packet<<discover<<chat<<invite<<play<<gameover<<msg;
                socket.send(packet,OppIP,clientPort);                                
                if (gameover){
                    gameList.erase(OppIP);
                    gameList.erase(remoteIP);
                    clientList.erase(OppIP);
                    clientList.erase(remoteIP);
                }
            }
            





        }



            
    }









}
