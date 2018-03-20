#include "Menu.hpp"
#include "network.h"
#include <sys/ioctl.h>
#include <termios.h>

Network::Network(){}

bool kbhit(){
	static const int STDIN = 0;
	static bool initialized =false;

	if(! initialized){
		termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &=~ICANON;
		tcsetattr(STDIN,TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized=true;
	}
	int bytesWaiting;
	ioctl(STDIN, FIONREAD, & bytesWaiting);
	return bytesWaiting>0;

}


void Network::setup(){
	  std::cout<<"Display name: "<<std::endl;
	  std::cin>>playerName;

	  socket.bind(45001); // client listen on port 45001
	  sendPort=45000;     // client sending with port 45000
	  socket.setBlocking(false);
	  std::string msg=playerName;
	  sf::Packet p;
	  std::string ip;
	  std::cout<<"IP of the server or search in (L)AN:"<<std::endl;
	  std::cin>>ip;
	  sf::IpAddress searchingIP;
	  if(ip.compare("L")==0 || ip.compare("l")==0){
		  searchingIP=sf::IpAddress::Broadcast;
	  }else{
		  searchingIP=ip;
	  }
	  std::cout<<"Looking server in "<<searchingIP.toString()<<std::endl;

	  bool discoverFlag=1, f=0;
	  p<<discoverFlag<<f<<f<<f<<f<<msg;
	  socket.send(p,searchingIP,sendPort);

	  std::map<std::string,std::string> serverList;

	  sf::Clock clock,inputClock;
	  bool loadingMsg=false;
	  
	  while(true){
		  if(!loadingMsg&&!inServer){
			  std::cout<<"Finding server..."<<std::endl;
			  loadingMsg=true;
		  }
		  sf::Packet packet;
		  if(socket.receive(packet,remoteIP,remotePort)==sf::Socket::Done){
			  std::string IPa=remoteIP.toString();
			  std::string msg;
			  bool discover,chat,invite,play,gameover;
			  packet>>discover>>chat>>invite>>play>>gameover>>msg;
			  //   std::cout<<"Server "<<msg<<" IP: "<<IPaddress<<std::endl;
			  if(discover && ! chat){
				  if(!inServer){
					  serverList.insert(std::make_pair(IPa,msg));
					  //Dummy server:
					  serverList.insert(std::make_pair("192.168.0.1","Dummy"));
				  }else{
					  msg="RESPONSE";
					  packet.clear();
					  packet<<discover<<chat<<invite<<play<<gameover<<msg;
					  socket.send(packet,IPAddress,sendPort);
				  }
			  }else if(discover && chat){
				  std::cout<<"Number of clients in session: "<<msg<<std::endl;

			  }else if (chat && !invite){
				  std::cout<<msg<<std::endl;
			  }else if (chat && invite){

				  std::cout<<msg<<" wants to play a game with you, (y)es or (n)o?"<<std::endl;
				  std::string buf;
				  std::cin>>buf;
				  if(buf=="y"){
					  std::cout<<"Accepted, waiting "<<msg<<" to choose a character"<<std::endl;

				  }else{
					  std::cout<<"Rejected, return to chatroom"<<std::endl;
				  }
				  chat=0;
				  play=1;
				  packet.clear();
				  buf+=msg;
				  packet<<discover<<chat<<invite<<play<<gameover<<buf;
				  socket.send(packet,IPAddress,sendPort);

			  }else if(invite && play){
				  std::cout<<"From opponent(invite&play): "<<std::endl;
				  char choice=msg[0];
				  msg.erase(msg.begin());
				  switch(choice){
				  case 'y':
					  std::cout<<msg<<" accepcted your inviation, let's select a character"<<std::endl;
					  characterSelectionScreen();
					  packet.clear();
					  msg=playerSelection+msg;
					  packet<<discover<<chat<<invite<<play<<gameover<<msg;
					  socket.send(packet,IPAddress,sendPort);
					  game=true;
					  break;
				  case 'n':
					  std::cout<<msg<<" rejected your inviation, return to chatroom"<<std::endl;
					  break;
				  case 'm':
					  marine=false;
					  game=true;
					  break;
				  case 'a':
					  marine=true;
					  game=true;
					  break;
				  default:
					  std::cerr<<"Packet (invite and play) contains incorrect character : "<<choice<<std::endl;
					  return ;
				  }

			  }else if(!invite && play){
					  //reconnect
					  marine=true;
					  game=true;
					  break;

			  }
		  }

		  if (game)
			  break;
		  if (inServer){
			  inputClock.restart();
			  std::string buf;
			  while(inputClock.getElapsedTime().asMilliseconds()<500){

				  if(kbhit()){
					  std::getline(std::cin,buf);
					  sf::Packet p;
					  bool f=0,t=1;
					  if(buf[0]=='/'){
						  if(buf[1]=='r'){
							  buf="REQUEST NUM CLIENTS";
							  bool discover=1,chat=1,invite=0,play=0,gameover=0;

							  p<<discover<<chat<<invite<<play<<gameover<<buf;

						  }else if(buf[1]=='i'){
							  buf.erase(0,3);

							  p<<f<<t<<t<<f<<f<<buf;

						  }//else wrong syntax
					  }else{
						  p<<f<<t<<f<<f<<f<<buf;
					  }
					  socket.send(p,IPAddress,sendPort);
					  break;

				  }
			  }
		  }

		  if(clock.getElapsedTime().asSeconds()>5 && !inServer){
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

				  int i=0;
				  for(itr=serverList.begin();itr!=serverList.end();++itr){
					  //std::cout<<i<<std::endl;
					  if(i==num){
						  IPAddress=itr->first;
						  serverName=itr->second;
						  packet.clear();
						  bool discoverFlag=1,chat=1,invite=0,play=0,gameover=0;
						  packet<<discoverFlag<<chat<<invite<<play<<gameover<<playerName;

						  socket.send(packet,IPAddress,sendPort);
						  std::cout<<"Joining "<<serverName<<" at "<<IPAddress<<std::endl;
						  inServer=true;
						  break;
					  }
					  i++;
				  }

			  }else{
				  p.clear();
				  bool discoverFlag=1,chat=1,invite=0,play=0,gameover=0;
				  p<<discoverFlag<<chat<<invite<<play<<gameover<<msg;

				  socket.send(p,searchingIP,sendPort);
				  clock.restart();
			  }

		  }//end of if clock
	  }//end of while


}// end of setup()

void Network::characterSelectionScreen(){
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
	                        if(m.selectedindex() == 1){
	                            playerSelection = "m";
	                        	marine=true;
	                        }else{
	                            playerSelection = "a";
	                            marine=false;
	                        }
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
}





//Send everything and set to data

void Network::sendAllData(sf::Vector2f& playerPos, sf::Vector2f& rectPos,
		int& playerRot, sf::Vector2f& projectilePos, int& projectileDir, float& projectileRot,
		InteractableManager* im,bool& update,bool& playerLoaded,int& playerHP){
	sf::Packet packet;
    bool discover=false, chat=false, invite=false,play=true;
    bool gameover=(playerHP<=0);
    packet << discover<< chat << invite << play << gameover;
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
    bool discover,chat,invite,play,gameover;

	if(socket.receive(packet, remoteIP, remotePort) == sf::Socket::Done){
        packet >> discover >> chat >> invite>>play >> gameover;
       packet >> playerPos.x>> playerPos.y >>rectPos.x>>rectPos.y>>projectilePos.x >> projectilePos.y;
       packet >> playerRot >> projectileDir >> projectileRot>>update;
       packet >> player2Loaded>>player2HP;

       if (gameover)
    	   player2HP=0;

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


