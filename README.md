# RealGraduation
인천대학교 졸업작품 TwistRun

외계행성에서 온 말랑이들이 벌이는 경쟁 레이싱 게임 TwistRun입니다.

언리얼엔진을 이용했으며 멀티플레이 게임입니다.

클라이언트가 C#웹 서버를 통해 서버PC에 존재하는 SQL에 접속하여 서버 IP를 가져간 후, 로비로 접속하는 방식입니다.

서버는 언리얼엔진의 데디케이트서버를 사용했으며 로비에 존재하는 인원들 중, 게임참여를 누르고 큐를 기다리는 유저들을 매칭시키는 방식으로 이루어집니다.

이 때, 데디케이트 서버 인스턴스가 자동적으로 생성되면서 방이 만들어지는데, 이 방에서 유저들이 레이싱을 즐기는 방식입니다.

웹 서버는 프로시져와 데이터베이스가 있어야 사용할 수 있으므로 이 리파지토리에는 넣지 않았습니다.
