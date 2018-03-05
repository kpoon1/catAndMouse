#include "Menu.hpp"
#include "network.h"

Network::Network(){}



void Network::setup(){
	//std::cout<<"(s)erver or (c)lient?"<<std::endl;
  	//std::cin>>playerSelection;
  	std::cout<<"Display name: "<<std::endl;
  	std::cin>>playerName;

// Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Catch Me");
    std::string ch;
    sf::Font font;

    Menu m(window.getSize().x, window.getSize().y);
    
    m.background();
    sf::Texture texture;
    texture.loadFromFile("img/cute_image.jpg");
    sf::Sprite sprite(texture);
    

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        
        while (window.pollEvent(event))
        {
            switch (event.type) {
                    
              case (sf::Event::KeyPressed):
                    
                switch (event.key.code){
                    case sf::Keyboard::Up:
                        m.MoveUp();
                        break;
                    case sf::Keyboard::Down:
                        m.MoveDown();
                        break;
                        
                    case sf::Keyboard::Return:
                        if(m.selectedindex() == 1)
                            playerSelection = "c";
                        else
                            playerSelection = "s";
                        window.close();
                        break;
                   	}
                    window.clear();
                    
                    window.draw(sprite);
                    m.draw(window);
                    // Update the window
                    window.display();
                    break;
                    
                case sf::Event::Closed:
                    window.close();
                    break;
            }
        }//end of while window poll event
    }//end of while window is open

  
	if (playerSelection=="s"){
	  std::cout<<"Waiting for client"<<std::endl;
	  marine=false;
	  socket.bind(45000); //server listen on port 45000
      sendPort=45001;
	  socket.setBlocking(false);
	  while(true){
	    sf::Packet packet;
	    if(socket.receive(packet,remoteIP,remotePort)==sf::Socket::Done){
	      std::string msg;
	      bool discoverFlag;
	      packet>>discoverFlag>>msg;
	      if(discoverFlag){
		std::cout<<"Received: "<<msg<<" from IP:"<<remoteIP.toString()<<std::endl;
		sf::Packet p;
		msg=playerName;
		p<<discoverFlag<<msg;
		socket.send(p,remoteIP,remotePort);
	      }else{
		std::cout<<"Starting the game with "<<msg<<std::endl;
		IPAddress=remoteIP.toString();
		break;
	      }
	    }
	    
	  }
	}else{
	  marine=true;
	  socket.bind(45001); // client listen on port 45001
	  sendPort=45000;     // client sending with port 45000
	  socket.setBlocking(false);
	  std::string msg="Player "+playerName;
	  sf::Packet p;
	  sf::IpAddress bc=sf::IpAddress::Broadcast;
	  bool discoverFlag=1;
	  p<<discoverFlag<<msg;
	  socket.send(p,bc,sendPort);

	  std::map<std::string,std::string> serverList;

	  sf::Clock clock;
	  bool loadingMsg=false;
	  
	  while(true){
		if(!loadingMsg){
			std::cout<<"Finding server..."<<std::endl;
			loadingMsg=true;
		}
	    sf::Packet packet;
	    if(socket.receive(packet,remoteIP,remotePort)==sf::Socket::Done){
	      std::string IPa=remoteIP.toString();
	      std::string msg;
	      bool discoverFlag;
	      packet>>discoverFlag>>msg;
	      //   std::cout<<"Server "<<msg<<" IP: "<<IPaddress<<std::endl;
	      serverList.insert(std::make_pair(IPa,msg));
			//Dummy server:
		  serverList.insert(std::make_pair("192.168.0.1","Dummy"));

	    }
	    

	    if(clock.getElapsedTime().asSeconds()>5){
	      printf("# \t ServerName \t IP Address\n");
	      std::map<std::string,std::string>::iterator itr;
	      int i=1;
	      for(itr=serverList.begin(); itr!=serverList.end();++itr){
		printf("%d \t %6s \t %-20s\n",i,itr->second.c_str(),itr->first.c_str());
		i++;
	      }
	      std::cout<<"Type the number to connect to the server, or \"r\" to refresh the list"<<std::endl;
	      char buff;
	      std::cin>>buff;
	      clock.restart();
			loadingMsg=false;	
	      if(buff!='r'){
			int num=(int)buff-49;
			std::cout<<"Typed: "<<num<<std::endl;
			int i=0;
			for(itr=serverList.begin();itr!=serverList.end();++itr){
				std::cout<<i<<std::endl;
		 		 if(i==num){
			   	 	IPAddress=itr->first;
					packet.clear();
					bool discoverFlag=false;
					packet<<discoverFlag<<playerName;

					socket.send(packet,IPAddress,sendPort);
					std::cout<<"Sending start game signal to "<<IPAddress<<std::endl;
					break;
			  	}
		  		i++;
			}
			break;
	      }else{
			p.clear();
			bool discoverFlag=1;
	 		p<<discoverFlag<<msg;
			socket.send(p,bc,sendPort);
		}
	
	    }//end of if clock
	  }//end of while
	}//end of if(playerselection)

}// end of setup()





//Send everything and set to data

void Network::sendAllData(sf::Vector2f& playerPos, sf::Vector2f& rectPos,
		int& playerRot, sf::Vector2f& projectilePos, int& projectileDir, float& projectileRot,
		InteractableManager* im,bool& update,bool& playerLoaded,int& playerHP){
	sf::Packet packet;
	packet << playerPos.x<< playerPos.y << rectPos.x << rectPos.y << projectilePos.x << projectilePos.y;
	packet << playerRot << projectileDir << projectileRot<<update;
	packet << playerLoaded<<playerHP;
	std::vector<interactable*> iaList=im->getIAList();
	for(int i=0;i<iaList.size();i++){
		bool isLoaded=false,occupied=false,isDeployed=false,activated=false,isOpen=false,isOccupied=false,lockerDoor=false;
		sf::Vector2f pos;
		std::string type=iaList[i]->getType();
		isLoaded=iaList[i]->getIsLoaded();
		if(type=="Item"||type=="DamageTrap"||type=="StickyTrap"){
			Item* it=dynamic_cast<Item*>(iaList[i]);
			occupied=it->getOccupied();
			pos=it->getSpritePos();
			if(type=="DamageTrap"||type=="StickyTrap"){
				trap* tp=dynamic_cast<trap*>(iaList[i]);
				isDeployed=tp->getIsDeployed();
				activated=tp->isActivated();
			}
		}
		if(type=="Table"||type=="Locker"){
			hidingPlace* hp=dynamic_cast<hidingPlace*>(iaList[i]);
			isOccupied=hp->getIsOccupied();
			lockerDoor=hp->getDoorOpen();
		}
		if(type=="Chest"){
			chest* ch=dynamic_cast<chest*>(iaList[i]);
			isOpen=ch->getIsOpen();
		}
		packet<<isLoaded<<pos.x<<pos.y<<occupied<<isDeployed<<activated<<isOpen<<isOccupied<<lockerDoor;
//		std::cout<<"Sent isDeployed: "<<isDeployed<<" Send posX: "<<pos.x<<" Send posY: "<<pos.y<<std::endl;

	}
//	if(packet.getDataSize()>40)
//		std::cout<<packet.getDataSize()<<std::endl;
	if(socket.send(packet, IPAddress, sendPort) != sf::Socket::Done){
        return;
	}
}

void Network::receiveAllData(sf::Vector2f& playerPos, sf::Vector2f& rectPos, int& playerRot,
		sf::Vector2f& projectilePos, int& projectileDir, float& projectileRot,
		InteractableManager* im, bool& player2Loaded, int& player2HP){
	sf::Packet packet;
	bool update;
	if(socket.receive(packet, remoteIP, remotePort) == sf::Socket::Done){
       packet >> playerPos.x>> playerPos.y >>rectPos.x>>rectPos.y>>projectilePos.x >> projectilePos.y;
       packet >> playerRot >> projectileDir >> projectileRot>>update;
       packet >> player2Loaded>>player2HP;

       bool isLoaded,occupied,isDeployed,activated,isOpen,isOccupied,lockerDoor;
       sf::Vector2f pos;
       std::vector<interactable*> iaList=im->getIAList();
       	for(int i=0;i<iaList.size();i++){
       		packet>>isLoaded>>pos.x>>pos.y>>occupied>>isDeployed>>activated>>isOpen>>isOccupied>>lockerDoor;
            if(update){
//				std::cout<<"Received isDeployed: "<<isDeployed<<" Received pos X: "<<pos.x<<" Y: "<<pos.y<<std::endl;
				std::string type=iaList[i]->getType();
				iaList[i]->setIsLoaded(isLoaded);
				if(type=="Item"||type=="DamageTrap"||type=="StickyTrap"){
					Item* it=dynamic_cast<Item*>(iaList[i]);
					it->setOccupied(occupied);
					it->setPosition(pos);
					if(type=="DamageTrap"||type=="StickyTrap"){
						trap* tp=dynamic_cast<trap*>(iaList[i]);
						tp->setIsDeployed(isDeployed);
						tp->setActivated(activated);
					}
				}
				if(type=="Table"||type=="Locker"){
					hidingPlace* hp=dynamic_cast<hidingPlace*>(iaList[i]);
					hp->setIsOccupied(isOccupied);
					hp->setDoorOpen(lockerDoor);
				}
				if(type=="Chest"){
					chest* ch=dynamic_cast<chest*>(iaList[i]);
					ch->setIsOpen(isOpen);
				}
            }//end if should Change
       	}

	}
}


