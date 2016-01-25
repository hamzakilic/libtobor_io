/*
 ============================================================================
 Name        : exampleProgram.c
 Author      : Hamza Kılıç
 Version     :
 Copyright   : Free To Use
 Description : Uses shared library to print greeting
               To run the resulting executable the LD_LIBRARY_PATH must be
               set to ${project_loc}/libtoborio/.libs
               Alternatively, libtool creates a wrapper shell script in the
               build directory of this program which can be used to run it.
               Here the script will be called exampleProgram.
 ============================================================================
 */


#include "em_io.h"


int test_all_pins(){
	em_log(EM_LOG_INFO,0,"hadi bakalım\n");
	  if(em_io_initialize(0))
		  return 1;
	  printf("initialized\n");
	int i=0;
	for(i=5;i<=8;++i)
	{
		//em_io_gpio_pull(i,EM_PULL_OFF);
		printf("%d out\n",i);
		//em_io_delay_microseconds(100000);
		 if(em_io_gpio_mode(i,EM_MODE_GPIO_OUT))
			  return 1;

		 printf("%d write\n",i);
		 //em_io_delay_microseconds(100000);
		 if(em_io_gpio_write(i,EM_GPIO_LOW))
		 		   return 1;

		 printf("%d write2\n",i);
		// em_io_delay_microseconds(100000);
	}


	i=0;
	int total_spin=0;
	//for(total_spin=0;total_spin<2;++total_spin)
	for(i=5;i<=8;++i){
        printf("turning on %d\n",i);
		if(em_io_gpio_write(i,EM_GPIO_HIGH))
		   return 1;
		// em_io_delay_microseconds(50000000);
	  // if(em_io_gpio_write(i,EM_GPIO_LOW))
	  // 	   return 1;


	  }


}
int test_read_pins(){
	em_log(EM_LOG_INFO,0,"hadi bakalım\n");
		  if(em_io_initialize(0))
			  return 1;
		int i=0;
		for(i=0;i<17;++i)
		{
			 if(em_io_gpio_mode(i,EM_MODE_GPIO_IN))
				  return 1;


		}
		em_io_gpio_mode(1,EM_MODE_GPIO_OUT);
		em_io_gpio_write(1,EM_GPIO_HIGH);
		em_uint8 read_val;
		for(i=0;i<100000;++i)
		{
			em_io_gpio_read(EM_GPIO_5,&read_val);
			if(read_val & EM_GPIO_LOW)
			fprintf(stdout,"read value is:low\n");
			else
				if(read_val & EM_GPIO_HIGH)
							fprintf(stdout,"read value is:high\n");
				else fprintf(stdout,"unknown value is readed:%d\n",read_val);

		}
		return 1;
}
int test_pull_up_down(){

	em_log(EM_LOG_INFO,0,"hadi bakalım\n");
			  if(em_io_initialize(0))
				  return 1;
			int i=0;

			if(em_io_gpio_mode(EM_GPIO_5,EM_MODE_GPIO_IN))
					  return 1;


			em_uint8 read_val;
			em_io_gpio_pull(EM_GPIO_5, EM_PULL_UP);
			sleep(1);
			for(i=0;i<100000;++i)
			{
				em_io_gpio_read(EM_GPIO_5,&read_val);
				if(read_val & EM_GPIO_LOW)
				fprintf(stdout,"read value 5 is:low\n");
				else
					if(read_val & EM_GPIO_HIGH)
								fprintf(stdout,"read value 5 is:high\n");
					else fprintf(stdout,"unknown value is readed:%d\n",read_val);



			}
			return 1;

}

em_uint32 test_all_up_read(){

	em_log(EM_LOG_INFO,0,"hadi bakalım\n");
				  if(em_io_initialize(0))
					  return 1;
				int i=0;
             for(i=0;i<17;++i)
				if(em_io_gpio_mode(i,EM_MODE_GPIO_IN))
						  return 1;
             em_uint8 val;
             for(i=0;i<17;++i)
             {
            	 em_io_gpio_read(i,&val);
            	 if(val& EM_GPIO_LOW)
            		 fprintf(stdout,"%d is low\n",i);
            	 else
            		 fprintf(stdout,"%d is high\n",i);
             }
             fprintf(stdout,"all up ********************press a key\n");
             getchar();

             for(i=0;i<17;++i)
            	 em_io_gpio_pull(i,EM_PULL_UP);
             for(i=0;i<17;++i)
                         {
                        	 em_io_gpio_read(i,&val);
                        	 if(val& EM_GPIO_LOW)
                        		 fprintf(stdout,"%d is low\n",i);
                        	 else
                        		 fprintf(stdout,"%d is high\n",i);
                         }


             fprintf(stdout,"all down ********************press a key\n");
             getchar();
                          for(i=0;i<17;++i)
                         	 em_io_gpio_pull(i,EM_PULL_DOWN);
                          for(i=0;i<17;++i)
                                      {
                                     	 em_io_gpio_read(i,&val);
                                     	 if(val& EM_GPIO_LOW)
                                     		 fprintf(stdout,"%d is low\n",i);
                                     	 else
                                     		 fprintf(stdout,"%d is high\n",i);
                                      }

}

em_uint32 test_events(){


	em_log(EM_LOG_INFO,0,"hadi bakalım t est events\n");
			  if(em_io_initialize(0))
				  return 1;
			  int i=0;
			  em_uint8 val=0;
			  for(i=0;i<17;++i){
			    em_io_gpio_read_event(i,&val);
			    if(val & EM_EVENT_DETECTED)
			    	fprintf(stdout,"%d pin event detected\n",i);
			    else
			    	fprintf(stdout,"%d pin event not detected\n",i);
			  }

			for(i=0;i<17;++i)
			{
				 if(em_io_gpio_mode(i,EM_MODE_GPIO_IN))
					  return 1;
				 em_io_gpio_pull(i,EM_PULL_DOWN);


			}
			em_io_gpio_mode(EM_GPIO_0,EM_MODE_GPIO_OUT);
			em_io_gpio_write(EM_GPIO_0,EM_GPIO_HIGH);
			fprintf(stdout,"executing set event\n");
			for(i=0;i<17;++i)
			em_io_gpio_set_event(i,EM_EVENT_RISING_EDGE_DETECT | EM_EVENT_FALLING_EDGE_DETECT | EM_EVENT_HIGH_DETECT | EM_EVENT_LOW_DETECT, EM_EVENT_DISABLE);
			em_log(EM_LOG_INFO,0,"executing set event 2\n");
			//for(i=0;i<17;++i)
			//em_io_gpio_set_event(i, EM_EVENT_HIGH_DETECT|EM_EVENT_LOW_DETECT|EM_EVENT_EDGE_DETECT ,EM_EVENT_ENABLE);
           // getchar();
			em_io_gpio_set_event(EM_GPIO_16, EM_EVENT_HIGH_DETECT  ,EM_EVENT_ENABLE);
			em_uint8 read_val;
			for(i=0;i<100000;++i)
			{

				/*em_io_gpio_read(EM_GPIO_16,&read_val);
				if(read_val & EM_GPIO_LOW)
				fprintf(stdout,"read value is:low\n");
				else
					if(read_val & EM_GPIO_HIGH)
								fprintf(stdout,"read value is:high\n");
					else fprintf(stdout,"unknown value is readed:%d\n",read_val);*/

                em_io_gpio_read_event(EM_GPIO_16,&val);
                if(val & EM_EVENT_DETECTED)
               			    	fprintf(stdout,"%d pin event detected %d \n",EM_GPIO_16,sizeof(em_uint32));
                else 		    	fprintf(stdout,"no event \n");

                em_io_delay_microseconds(500);

			}
			return 1;
}

int test_time(){
	  if(em_io_initialize(0))
					  return 1;
	  em_uint64 time=0;
	  int counter=0;
	  for(counter=0;counter <10 ;++counter){
		  em_io_current_time(&time);
		  fprintf(stdout,"time is:%lld microsecond\n ",time);
		  em_io_delay_microseconds(1*100000);
	  }
}

int test_mini_uart(){
	if(em_io_initialize(0))
						  return 1;
	//em_raspi_uart_extra_status();
		int i=270;

		em_io_mini_uart_start(EM_MINI_UART_RECEIVE_ENABLE| EM_MINI_UART_TRANSMIT_ENABLE|EM_MINI_UART_DATA_8BIT_ENABLE,115200,250);
		em_io_delay_loops(100);
		em_log(EM_LOG_INFO,0,"******\n");
		//em_raspi_uart_extra_status();
	    const char * value="hamza calisti";

	    em_uint8 read=0;
	        while(1){

	        	if(read){
	        		if(em_io_mini_uart_write(read)==EM_SUCCESS)  	   read=0;

	        	}else
	        	if(em_io_mini_uart_read(&read)==EM_SUCCESS)

	        	if(em_io_mini_uart_write(read)==EM_SUCCESS){
	        	   read=0;
	        	}


	        }


	    em_raspi_uart_extra_status();
		return EM_SUCCESS;

}




int test_uart(){
	if(em_io_initialize(0))
						  return 1;
	em_io_uart_start(EM_UART_RECEIVE_ENABLE|EM_UART_TRANSMIT_ENABLE| EM_UART_DATA_8BIT_ENABLE| EM_UART_PARITY_ENABLE|EM_UART_FIF0_ENABLE ,9600);

		int i=0;
		for(i=0;i<10;++i){
			em_io_uart_write('a');
		}
		em_uint8 read=0;
		while(1){

			if(em_io_uart_read(&read)==EM_SUCCESS){
				em_io_uart_write(read);

			}
		}

		return EM_SUCCESS;

}




int test_pwm(){
	if(em_io_initialize(0))
							  return 1;

	em_raspi_pwm_start(0, EM_PWM_MODE_DEFAULT |EM_PWM_USE_FIF0,1024,256,0);
		int i=0;


		em_raspi_pwm_status();
	    em_uint32 val_temp=0;
		while(1)
		{
			if(em_raspi_pwm_write(0,val_temp++))
				val_temp--;

			if(val_temp>=1024)
				val_temp=0;
			em_raspi_busy_wait(10);
		}
}

int test_i2c(){

	if(em_io_initialize(0))
							  return 1;
	//testing atmel 24c02
	em_uint16 slave=0x51;
	em_uint8 s=0x10;
	em_raspi_i2c_start(EM_USE_BSC1,0,0);
	for(s=0x00;s<0xFF;++s){
		em_log(EM_LOG_INFO,0,"address is %#010x\n",s);

	    em_uint8 data[]={s,s};

	    if(em_raspi_i2c_write(EM_USE_BSC1,slave,data,2,EM_TIMEOUT_ONE_SECOND)){
	   	em_log(EM_LOG_INFO,0,"birinci yazilamadi\n");
	   	break;
	    }else em_log(EM_LOG_INFO,0,"birinci yazildi \n");
	    em_io_busy_wait(0.001*1000000);

		if(em_raspi_i2c_write(EM_USE_BSC1,slave,data,1,EM_TIMEOUT_ONE_SECOND)){
		   	em_log(EM_LOG_INFO,0,"ikinci yazilamadi\n");
		    	break;
		}else{
			em_log(EM_LOG_INFO,0,"ikinci yazildi\n");
		}
		 em_io_busy_wait(0.001*1000000);
		em_uint8 data2[]={0x0,0};
		em_uint32 lenght=1;

		if(em_raspi_i2c_read(EM_USE_BSC1,slave,data2,lenght,EM_TIMEOUT_ONE_SECOND)){
			em_log(EM_LOG_INFO,0,"okunamadı\n");
			break;
		}
		else{
			em_log(EM_LOG_INFO,0,"lenght=%u data=%#010x\n",lenght,data2[0]);
		}
	}

}


int test_pca9685_pwm_driver(){
	if(em_io_initialize(0))
		return 1;
#define MAX_PWM 148;
		//int frequency_divider=250000000/MAX_PWM;
		em_uint8 slave=0x40;
		int read=0x01;
		int write=0xFE;
		int fixed=0x80;

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6+5*4
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD

		em_log(EM_LOG_FATAL,0,"starting i2c\n");
		em_io_i2c_start(EM_USE_BSC1,0,0);
		em_log(EM_LOG_FATAL,0,"started i2c\n");
		//int frequency_scale= (int)(25000000.0f / (4096 * 50) -1.0f);
		int frequency_scale= (int)(25000000.0f /(4096*200)-1.0f);
		em_log(EM_LOG_FATAL,0,"frequency scale is %u\n",frequency_scale);


		em_uint8 oldmod=0;
		em_uint32 length=1;
		em_uint8 data[]={49};
		if(em_io_i2c_write(EM_USE_BSC1,slave,data,length,EM_TIMEOUT_ONE_SECOND)){
					em_log(EM_LOG_FATAL,0,"mod write error\n");
								return 1;
				}
		if(em_io_i2c_read(EM_USE_BSC1,slave,&oldmod,length,EM_TIMEOUT_ONE_SECOND)){
			em_log(EM_LOG_FATAL,0,"mod read error\n");
			return 1;
		}
		em_log(EM_LOG_FATAL,0,"old mode is %u\n",oldmod);
		/*em_uint8 newMode=(oldmod &0x7F) |0x10;//sleep
		em_uint8 data2[2]={PCA9685_MODE1,newMode};
		if(em_io_i2c_write(EM_USE_BSC1,slave,data2,2)){
			em_log(EM_LOG_FATAL,0,"mod write error1\n");
						return 1;
		}*/
		em_uint8 data2[2]={0,0};
		data2[0]=PCA9685_PRESCALE;
		data2[1]=frequency_scale;
		if(em_io_i2c_write(EM_USE_BSC1,slave,data2,2,EM_TIMEOUT_ONE_SECOND)){
					em_log(EM_LOG_FATAL,0,"frequency write error\n");
								return 1;
				}
		em_io_delay_loops(5000);
		data2[0]=PCA9685_MODE1;
	    data2[1]=0xa1;
	    if(em_io_i2c_write(EM_USE_BSC1,slave,data2,2,EM_TIMEOUT_ONE_SECOND)){
	    			em_log(EM_LOG_FATAL,0,"mod write error3\n");
	    						return 1;
	    }
	    em_io_delay_loops(5000);

	    	    		if(em_io_i2c_read(EM_USE_BSC1,slave,&oldmod,length,EM_TIMEOUT_ONE_SECOND)){
	    	    			em_log(EM_LOG_FATAL,0,"mod read error\n");
	    	    			return 1;
	    	    		}
	    	    		em_log(EM_LOG_FATAL,0,"old mode is %u\n",oldmod);
                          em_uint32 min=700;
                          em_uint32 max=2000;


	    	    		 em_uint8 datapwm[5];

	    	    			    	    	    	    	em_uint32 on=0;//4095-i;
	    	    			    	    	    	    	em_uint32 off=max;
	    	    			    	    	    	    	datapwm[0]=LED0_ON_L;
	    	    			    	    	    	    	datapwm[1]=on;// & 0xFF;
	    	    			    	    	    	    	datapwm[2]=(on>>8);//& 0xf;
	    	    			    	    	    	    	datapwm[3]=off ;//& 0xFF;
	    	    			    	    	    	    	datapwm[4]=(off >>8);//& 0xf;
	    	    			    	    	    	    	 if(em_io_i2c_write(EM_USE_BSC1,slave,datapwm,5,EM_TIMEOUT_ONE_SECOND)){
	    	    			    	    	    	    		    	    			em_log(EM_LOG_FATAL,0,"pwm write error\n");
	    	    			    	    	    	    		    	    						return 1;
	    	    			    	    	    	    	 }
	    	    			    		    	    			em_log(EM_LOG_INFO,0,"open motor\n");
	    	    			    		    	    			getchar();
	    	    			    		    	    			em_io_delay_microseconds(3000000);
	    	    			    		    	    			off=min;
	    	    			    		    	    			datapwm[0]=LED0_ON_L;
	    	    			    		    	    				    	    			    	    	    	    	datapwm[1]=on;// & 0xFF;
	    	    			    		    	    				    	    			    	    	    	    	datapwm[2]=(on>>8);//& 0xf;
	    	    			    		    	    				    	    			    	    	    	    	datapwm[3]=off ;//& 0xFF;
	    	    			    		    	    				    	    			    	    	    	    	datapwm[4]=(off >>8);//& 0xf;
	    	    			    		    	    				    	    			    	    	    	    	 if(em_io_i2c_write(EM_USE_BSC1,slave,datapwm,5,EM_TIMEOUT_ONE_SECOND)){
	    	    			    		    	    				    	    			    	    	    	    		    	    			em_log(EM_LOG_FATAL,0,"pwm write error\n");
	    	    			    		    	    				    	    			    	    	    	    		    	    						return 1;
	    	    			    		    	    				    	    			    	    	    	    	 }
	    	    			    		    	    				    	    			    	    	    	    	 em_io_delay_microseconds(1000000);
getchar();

getchar();







	    	    /*em_log(EM_LOG_FATAL,0,"starting\n");

	    	                    em_uint32 values[]={5000,-5000};
	    	                    em_uint32 i;
	    	                    em_uint32 temp=0;
	    	                    for(temp=min+50;temp<max;temp++){

	    	    	    	    em_uint8 datapwm[5];
	    	    	    	    	em_uint32 on=0;
	    	    	    	    	em_uint32 off=temp;
	    	    	    	    	datapwm[0]=LED0_ON_L;
	    	    	    	    	datapwm[1]=on;// & 0xFF;
	    	    	    	    	datapwm[2]=(on>>8);//& 0xf;
	    	    	    	    	datapwm[3]=off ;//& 0xFF;
	    	    	    	    	datapwm[4]=(off >>8);//& 0xf;
	    	    	    	    	 if(em_io_i2c_write(EM_USE_BSC1,slave,datapwm,5)){
	    	    	    	    		    	    			em_log(EM_LOG_FATAL,0,"pwm write error\n");
	    	    	    	    		    	    						return 1;
	    	    	    	    	 }
	    	    	    	    	 em_io_delay_microseconds(10000);
	    	    	    	    	 em_log(EM_LOG_FATAL,0,"%u\n",temp);

	    	    	    	    	 }
	    	                    for(temp=max;temp>min+50;temp--){

	    	                    	    	    	    	    em_uint8 datapwm[5];
	    	                    	    	    	    	    	em_uint32 on=0;
	    	                    	    	    	    	    	em_uint32 off=temp;
	    	                    	    	    	    	    	datapwm[0]=LED0_ON_L;
	    	                    	    	    	    	    	datapwm[1]=on;// & 0xFF;
	    	                    	    	    	    	    	datapwm[2]=(on>>8);//& 0xf;
	    	                    	    	    	    	    	datapwm[3]=off ;//& 0xFF;
	    	                    	    	    	    	    	datapwm[4]=(off >>8);//& 0xf;
	    	                    	    	    	    	    	 if(em_io_i2c_write(EM_USE_BSC1,slave,datapwm,5)){
	    	                    	    	    	    	    		    	    			em_log(EM_LOG_FATAL,0,"pwm write error\n");
	    	                    	    	    	    	    		    	    						return 1;
	    	                    	    	    	    	    	 }
	    	                    	    	    	    	    	 em_io_delay_microseconds(10000);
	    	                    	    	    	    	    	 em_log(EM_LOG_FATAL,0,"%u\n",temp);

	    	                    	    	    	    	    	 }*/
	    	                    int temp=min;
	    	                    while(1){
	    	                    	 em_uint8 datapwm[5];
	    	                    		    	                    	    	    	    	    	em_uint32 on=0;
	    	                    		    	                    	    	    	    	    	em_uint32 off=temp;
	    	                    		    	                    	    	    	    	    	datapwm[0]=LED0_ON_L;
	    	                    		    	                    	    	    	    	    	datapwm[1]=on;// & 0xFF;
	    	                    		    	                    	    	    	    	    	datapwm[2]=(on>>8);//& 0xf;
	    	                    		    	                    	    	    	    	    	datapwm[3]=off ;//& 0xFF;
	    	                    		    	                    	    	    	    	    	datapwm[4]=(off >>8);//& 0xf;
	    	                    		    	                    	    	    	    	    	 if(em_io_i2c_write(EM_USE_BSC1,slave,datapwm,5,EM_TIMEOUT_ONE_SECOND)){
	    	                    		    	                    	    	    	    	    		    	    			em_log(EM_LOG_FATAL,0,"pwm write error\n");
	    	                    		    	                    	    	    	    	    		    	    						return 1;
	    	                    		    	                    	    	    	    	    	 }

	    	                    		    	                    	    	    	    	    	 em_log(EM_LOG_FATAL,0,"%u\n",temp);
	    	                    		    	                    	    	    	    	    	 //temp++;
	    	                    		    	                    	    	    	    	    	int get= getchar();
	    	                    		    	                    	    	    	    	    	if(get=='a')
	    	                    		    	                    	    	    	    	    		temp++;
	    	                    		    	                    	    	    	    	    	else if(get=='z')temp--;
	    	                    		    	                    	    	    	    	    	 //em_io_delay_microseconds(1000000);


	    	                    }


	    	   /* while(1){
	    	    em_uint32 i;
	    	    em_uint8 datapwm[5];
	    	    for(i=-5000;i<=5000;++i){
	    	    	//getchar();
	    	    	em_uint32 on=0;//4095-i;
	    	    	em_uint32 off=4095*(i+5000)/5000.0f;
	    	    	datapwm[0]=LED0_ON_L;
	    	    	datapwm[1]=on;// & 0xFF;
	    	    	datapwm[2]=(on>>8);//& 0xf;
	    	    	datapwm[3]=off ;//& 0xFF;
	    	    	datapwm[4]=(off >>8);//& 0xf;
	    	    	 if(em_io_i2c_write(EM_USE_BSC1,slave,datapwm,5)){
	    	    		    	    			em_log(EM_LOG_FATAL,0,"pwm write error\n");
	    	    		    	    						return 1;
	    	    		    	    }
	    	    	 //break;
	    	    	 em_io_delay_microseconds(10000);

	    	    	 em_log(EM_LOG_FATAL,0,"pwm is %u\n",i);
	    	    }
	    	    }*/







}


int main(void) {
	test_uart();


  return 0;
}
